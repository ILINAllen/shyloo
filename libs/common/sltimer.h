/********************************************************************
	created:	2016/07/29
	created:	29:7:2016   15:08
	filename: 	d:\Projects\shyloo\libs\common\sltimer.h
	file path:	d:\Projects\shyloo\libs\common
	file base:	sltimer
	file ext:	h
	author:		ddc
	
	purpose:	��ʱ��
*********************************************************************/

#ifndef _SL_TIMER_H_
#define _SL_TIMER_H_
#include "slbase.h"
namespace sl
{
	class TimersBase;
	class TimeBase;

	class TimerHandle
	{
	public:
		explicit TimerHandle(TimeBase* pTime = NULL):m_pTime(pTime){}
		bool IsSet() const {return m_pTime != NULL;}
		TimeBase* GetTime() const {return m_pTime;}
		void Cancel();

		friend bool operator==(TimerHandle tHandle1, TimerHandle tHandle2);
	
	
	private:
		TimeBase* m_pTime;
	};

	inline bool operator==(TimerHandle tHandle1, TimerHandle tHandle2)
	{
		return tHandle1.m_pTime == tHandle2.m_pTime;
	}


	class TimerHandler
	{
	public:
		TimerHandler():m_iNumTimesRegistered(0){}
		virtual ~TimerHandler(){}
		virtual void HandlerTimeOut() = 0; 

	protected:
		virtual void OnRelease(TimerHandle handle, void* pUser){}
	private:
		friend class TimeBase;

		void IncTimerRegisterCount() {++m_iNumTimesRegistered;}
		void DecTimerRegisterCount() {--m_iNumTimesRegistered;}

		void Release(TimerHandle handle, void* pUser)
		{
			this->DecTimerRegisterCount();
			this->OnRelease(handle, pUser);
		}

	private:
		int			m_iNumTimesRegistered;
	};


	class TimeBase
	{
	public:
		TimeBase(TimersBase& Owner, TimerHandler* pHandler,
			void* pUserData);
		virtual ~TimeBase();

		void Cancel();

		void* GetUserData() const {return m_pUserData;}

		bool IsCancelled() const {return m_stState == TIME_CANCELLED;}
		bool IsExecting() const {return m_stState == TIME_EXECUTING;}
	protected:
		enum TimeState
		{
			TIME_EXECUTING,
			TIME_PENDING,
			TIME_CANCELLED
		};

		TimersBase&		m_Owner;
		TimerHandler*	m_pHandler;
		void*			m_pUserData;
		TimeState		m_stState;
	};

	class TimersBase
	{
	public:
		virtual void OnCancel() = 0;
	};

	template<class TIME_STAMP>
	class TimersT: public TimersBase
	{
	public:
		typedef TIME_STAMP TimeStamp;

		TimersT();
		virtual ~TimersT();

		inline UINT32 Size() const {return m_TimeQueue.Size();}
		inline bool Empty() const {return m_TimeQueue.Empty();}

		int Process(TimeStamp now);
		bool legal(TimerHandle handle) const;

		TimeStamp nextExp(TimeStamp now) const;
		void Clear(bool shouldCallCancel = true);

		bool GetTimerInfo(TimerHandle handle, TimeStamp& time, TimeStamp& interval,
			void*& pUserData) const;

		TimerHandle Add(TimeStamp startTime, TimeStamp interval, TimerHandler* pHandler, void* pUser);


	private:
		typedef std::vector<sl::TimeBase*> TimeContainer;
		TimeContainer		m_TimeContainer;

		void PurgeCanelledTimes();
		void OnCancel();

		class Time: public TimeBase
		{
		public:
			Time(TimersBase& owner, TimeStamp startTime, TimeStamp interval,
				TimerHandler* pHandler, void* pUserData);
		private:
			Time(const Time&);
			Time& operator=(const Time&);
		public:
			TimeStamp	GetTime() const {return m_Time; }
			TimeStamp	GetInterval() const {return m_Interval;}

			void TriggerTimer();
		private:
			TimeStamp			m_Time;
			TimeStamp			m_Interval;
		};

		class Comparator
		{
		public:
			bool operator()(const Time* a, const Time* b)
			{
				return a->GetTime() > b->GetTime();
			}
		};

		class PriorityQueue
		{
		public:
			typedef std::vector<Time*> Container;

			typedef typename Container::value_type value_type;
			typedef typename Container::size_type  size_type;

			bool Empty() const {return m_TimeContainer.empty();}
			size_type Size() const {return m_TimeContainer.size();}

			const value_type& Top() const {return m_TimeContainer.front();}

			void Push(const value_type& x)
			{
				m_TimeContainer.push_back(x);
				std::push_heap(m_TimeContainer.begin(), m_TimeContainer.end(), Comparator());
			}

			void Pop()
			{
				std::pop_heap(m_TimeContainer.begin(), m_TimeContainer.end(),Comparator());
				m_TimeContainer.pop_back();
			}

			Time* UnsafePopBack()
			{
				Time* pTime = m_TimeContainer.back();
				m_TimeContainer.pop_back();
				return pTime;
			}

			Container& GetContainer() {return m_TimeContainer;}

			void MakeHeap()
			{
				std::make_heap(m_TimeContainer.begin(), m_TimeContainer.end(),Comparator());
			}
		private:
			Container		m_TimeContainer;
		};

		PriorityQueue	m_TimeQueue;
		Time*			m_pProcessingNode;
		TimeStamp		m_lastProcessTime;
		int				m_iNumCanceled;

		TimersT(const TimersT&);
		TimersT& operator=(const TimersT&);

	};

}// namespace sl
#endif