#pragma once

#include <AvoGUI.hpp>

class Worker : public AvoGUI::Component
{
private:
	std::condition_variable m_needsToWakeUpConditionVariable;
	std::mutex m_needsToWakeUpMutex;
	std::atomic<bool> m_needsToWakeUp{ false };

	std::mutex m_callbackQueueMutex;
	std::deque<std::function<void()>> m_callbackQueue;

	std::thread m_thread{ &Worker::run };

	void run()
	{
		while (getParent())
		{
			while (m_callbackQueue.size())
			{
				m_callbackQueue.front()(); // funky
				m_callbackQueue.pop_front();
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

	Worker()
	{
		parentChangeListeners += [this](Component* oldParent) {
			if (oldParent && !getParent())
			{
				m_thread.join();
			}
		};
	}
};
