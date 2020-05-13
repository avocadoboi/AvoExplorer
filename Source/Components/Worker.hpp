#pragma once

#include <AvoGUI.hpp>

#include <condition_variable>

class Worker : public AvoGUI::Component
{
private:
	std::condition_variable m_needsToWakeUpConditionVariable;
	std::mutex m_needsToWakeUpMutex;
	std::atomic<bool> m_needsToWakeUp{ false };

	std::mutex m_callbackQueueMutex;
	std::deque<std::function<void()>> m_callbackQueue;

	std::thread m_thread{ &Worker::run, this };

	void run()
	{
		while (getParent())
		{
			m_needsToWakeUp = false;
			while (m_callbackQueue.size())
			{
				m_callbackQueueMutex.lock();
				auto callback = m_callbackQueue.front();
				m_callbackQueue.pop_front();
				m_callbackQueueMutex.unlock();

				// We call it outside of the lock to avoid deadlock, because the callback might be adding another 
				// callback to the worker and therefore locking the callback queue mutex as well.
				callback();
			}
			
			if (!m_needsToWakeUp)
			{
				std::unique_lock<std::mutex> mutexLock(m_needsToWakeUpMutex);
				m_needsToWakeUpConditionVariable.wait(mutexLock, [this]() { return (bool)m_needsToWakeUp; });
			}
		}
	}

public:
	void requestCallback(std::function<void()> const& p_callback)
	{
		m_callbackQueueMutex.lock();
		m_callbackQueue.push_back(p_callback);
		m_callbackQueueMutex.unlock();

		if (!m_needsToWakeUp)
		{
			m_needsToWakeUpMutex.lock();
			m_needsToWakeUp = true;
			m_needsToWakeUpMutex.unlock();
			m_needsToWakeUpConditionVariable.notify_one();
		}
	}

	Worker(Component* p_parent) :
		Component(p_parent)
	{
		parentChangeListeners += [this](Component* oldParent) {
			if (oldParent && !getParent())
			{
				m_needsToWakeUpMutex.lock();
				m_needsToWakeUp = true;
				m_needsToWakeUpMutex.unlock();
				m_needsToWakeUpConditionVariable.notify_one();
				m_thread.join();
			}
		};
	}
};
