#ifndef MESSAGING_H
#define MESSAGING_H

#include <panda/core.h>

// https://github.com/NoAvailableAlias/nano-signal-slot
// Modified it a little bit for my needs (put some classes as friends)
#include <nano-signal-slot/nano_function.hpp>
#include <nano-signal-slot/nano_observer.hpp>

namespace panda
{

	namespace msg
	{
		class Observer;

		// This class describes a signal
		// Observers can connect to it, and will be notified when it is run
		template <typename Ret> class Signal;
		template <typename Ret, typename... Args>
		class Signal<Ret(Args...)> : public Nano::Observer
		{
		public:
			using Delegate = Nano::Function<Ret(Args...)>;

			template <typename... Args>
			void run(Args&&... args)
			{
				Nano::Observer::onEach<Delegate>(std::forward<Args>(args)...);
			}

			template <typename Accumulate, typename... Args>
			void accumulate(Accumulate&& accumulate, Args&&... args)
			{
				Nano::Observer::onEach_Accumulate<Delegate, Accumulate>
					(std::forward<Accumulate>(accumulate), std::forward<Args>(args)...);
			}

		private:
			template <typename T> friend class Wrapper;
		};

		namespace details
		{
			template <typename Ret> class Wrapper;
			template <typename Ret, typename... Args>
			class Wrapper<Ret(Args...)>
			{
			public:
				Wrapper(Signal<Ret(Args...)>& signal, Observer* observer)
					: m_signal(signal)
					, m_observer(observer)
				{}

				using Delegate = Nano::Function<Ret(Args...)>;

				template <typename L> 
				void connect(L* instance)
				{ m_signal.insert(Delegate::template bind(instance), m_observer); }

				template <typename L> 
				void connect(L& instance)
				{ connect(std::addressof(instance)); }

				template <Ret(*fun_ptr)(Args...)> 
				void connect()
				{ m_signal.insert(Delegate::template bind<fun_ptr>(), m_observer); }

				template <typename T, Ret(T::*mem_ptr)(Args...)> 
				void connect(T* instance)
				{ m_signal.insert(Delegate::template bind<T, mem_ptr>(instance), m_observer); }

				template <typename T, Ret(T::*mem_ptr)(Args...) const> 
				void connect(T* instance)
				{ m_signal.insert(Delegate::template bind<T, mem_ptr>(instance), m_observer); }

				template <typename T, Ret(T::*mem_ptr)(Args...)> 
				void connect(T& instance)
				{ connect<T, mem_ptr>(std::addressof(instance)); }

				template <typename T, Ret(T::*mem_ptr)(Args...) const>
				void connect(T& instance)
				{ connect<T, mem_ptr>(std::addressof(instance)); }

			private:
				Signal<Ret(Args...)>& m_signal;
				Observer* m_observer;
			};

		}

		// This class is used to connect to signals
		// When released, it unregister itself from the signals it was connected to
		class PANDA_CORE_API Observer : public Nano::Observer
		{
		public:
			template <typename Ret, typename... Args>
			details::Wrapper<Ret(Args...)> get(Signal<Ret(Args...)>& signal)
			{ return details::Wrapper<Ret(Args...)>(signal, this); }
		};

	} // namespace msg

} // namespace panda

#endif // MESSAGING_H
