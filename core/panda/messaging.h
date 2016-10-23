#ifndef MESSAGING_H
#define MESSAGING_H

#include <panda/core.h>

// I copied and simplified the Nano-Signal-Slot library
// https://github.com/NoAvailableAlias/nano-signal-slot

#include <cstdint>
#include <array>
#include <forward_list>

namespace panda
{

	namespace msg
	{
		class BaseObserver;
		template <typename Ret> class Signal;

		namespace details
		{
			template <typename Ret> class Wrapper;

			using DelegateKey = std::array<std::uintptr_t, 2>;

			template <typename RT> class Function;
			template <typename RT, typename... Args>
			class Function<RT(Args...)>
			{
				using Thunk = RT (*)(void*, Args...);

				friend class BaseObserver;

				void* m_this_ptr; // instance pointer
				Thunk m_stub_ptr; // free function pointer

				Function(void* this_ptr, Thunk stub_ptr) :
					m_this_ptr(this_ptr), m_stub_ptr(stub_ptr) {}

				Function(DelegateKey delegate_key) :
					m_this_ptr(reinterpret_cast<void*>(delegate_key[0])),
					m_stub_ptr(reinterpret_cast<Thunk>(delegate_key[1])) {}

				public:

				template <RT (*fun_ptr) (Args...)>
				static inline Function bind()
				{
					return { nullptr, [](void* /*NULL*/, Args... args)
						{ return (*fun_ptr)(std::forward<Args>(args)...); } };
				}
				template <typename T, RT (*fun_ptr) (T*, Args...)>
				static inline Function bind(T* pointer)
				{
					return { pointer, [](void* this_ptr, Args... args)
						{ return (*fun_ptr)(static_cast<T*>(this_ptr), std::forward<Args>(args)...); } };
				}
				template <typename T, RT (T::* mem_ptr) (Args...)>
				static inline Function bind(T* pointer)
				{
					return { pointer, [](void* this_ptr, Args... args)
						{ return (static_cast<T*>(this_ptr)->*mem_ptr) (std::forward<Args>(args)...); } };
				}
				template <typename T, RT (T::* mem_ptr) (Args...) const>
				static inline Function bind(T* pointer)
				{
					return { pointer, [](void* this_ptr, Args... args)
						{ return (static_cast<T*>(this_ptr)->*mem_ptr) (std::forward<Args>(args)...); } };
				}
				template <typename L>
				static inline Function bind(L* pointer)
				{
					return { pointer, [](void *this_ptr, Args... args)
						{ return (static_cast<L*>(this_ptr)->operator()(std::forward<Args>(args)...)); }};
				}
				inline operator DelegateKey() const
				{
					return
					{{
						reinterpret_cast<std::uintptr_t>(m_this_ptr),
						reinterpret_cast<std::uintptr_t>(m_stub_ptr)
					}};
				}
				template <typename... Uref>
				inline RT operator() (Uref&&... args)
				{
					return (*m_stub_ptr)(m_this_ptr, std::forward<Uref>(args)...);
				}
			};

			// Intermediate class used to connect to a signal and setting the caller as the listener
			template <typename Ret, typename... Args>
			class Wrapper<Ret(Args...)>
			{
			public:
				Wrapper(Signal<Ret(Args...)>& signal, BaseObserver* observer)
					: m_signal(signal)
					, m_observer(observer)
				{}

				using Delegate = details::Function<Ret(Args...)>;

				template <typename L> 
				void connect(L* instance)
				{ m_signal.insert(Delegate::template bind(instance), m_observer); }

				template <typename L> 
				void connect(L& instance)
				{ connect(std::addressof(instance)); }

				template <Ret(*fun_ptr)(Args...)> 
				void connect()
				{ m_signal.insert(Delegate::template bind<fun_ptr>(), m_observer); }

				template <typename T, Ret(*fun_ptr)(T*, Args...)> 
				void connect(T* instance)
				{ m_signal.insert(Delegate::template bind<T, fun_ptr>(instance), m_observer); }

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
				BaseObserver* m_observer;
			};

		} // namespace details

		// This class stores a list of listeners
		class PANDA_CORE_API BaseObserver
		{
		public:
			BaseObserver() = default;
			BaseObserver(const BaseObserver& other) = delete; // non construction-copyable
			BaseObserver& operator=(const BaseObserver&) = delete; // non copyable

			~BaseObserver()
			{ removeAll(); }

		private:
			template <typename T> friend class Signal;
			template <typename T> friend class details::Wrapper;

			void insert(const details::DelegateKey& key, BaseObserver* obs)
			{ m_list.emplace_front(key, obs); }

			void insert(const details::DelegateKey& key)
			{ m_list.emplace_front(key, this); }

			void remove(const details::DelegateKey& key)
			{
				m_list.remove_if([&key](const DelegateKeyObserverPair& p) {
					return p.first == key;
				});
			}

			void removeAll()
			{
				for (const auto& p : m_list)
				{
					// Notify the listening Observer
					if (p.second != this)
						p.second->remove(p.first);
				}

				m_list.clear();
			}

			template <typename Delegate, typename... Uref>
			void onEach(Uref&&... args) const
			{
				for (auto& p : m_list)
					// Perfect forward and emit
					Delegate(p.first)(std::forward<Uref>(args)...);
			}

			template <typename Delegate, typename Accumulate, typename... Uref>
			void onEach_Accumulate(Accumulate&& accumulate, Uref&&... args) const
			{
				for (auto& p : m_list)
					// Perfect forward, emit, and accumulate the return value
					accumulate(Delegate(p.first)(std::forward<Uref>(args)...));
			}

			using DelegateKeyObserverPair = std::pair<details::DelegateKey, BaseObserver*>;
			std::forward_list<DelegateKeyObserverPair> m_list;
		};

		// This class is used to connect to signals
		// When released, it unregister itself from the signals it was connected to
		class PANDA_CORE_API Observer : public BaseObserver
		{
		public:
			template <typename Ret, typename... Args>
			details::Wrapper<Ret(Args...)> get(Signal<Ret(Args...)>& signal)
			{ return details::Wrapper<Ret(Args...)>(signal, this); }
		};

		// This class describes a signal
		// Observers can connect to it, and will be notified when it is run
		template <typename Ret, typename... Args>
		class Signal<Ret(Args...)> : public BaseObserver
		{
		public:
			using Delegate = details::Function<Ret(Args...)>;

			template <typename... Args>
			void run(Args&&... args)
			{ BaseObserver::onEach<Delegate>(std::forward<Args>(args)...); }

			template <typename Accumulate, typename... Args>
			void accumulate(Accumulate&& accumulate, Args&&... args)
			{ 
				BaseObserver::onEach_Accumulate<Delegate, Accumulate>
					(std::forward<Accumulate>(accumulate), std::forward<Args>(args)...);
			}

		private:
			template <typename T> friend class details::Wrapper;

			void insert(const details::DelegateKey& key, BaseObserver* obs)
			{
				obs->insert(key, this);
				BaseObserver::insert(key, obs);
			}
		};

	} // namespace msg

} // namespace panda

#endif // MESSAGING_H
