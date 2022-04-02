#ifndef VMC_SINGLETON_H
#define VMC_SINGLETON_H

#include <memory>

template<class T>
class Singleton
{
	public:
		~Singleton() = default;

		Singleton(const Singleton &) = delete;
		Singleton(Singleton &&) = delete;

		Singleton &operator=(const Singleton &) = delete;
		Singleton &operator=(Singleton &&) = delete;

		static T *Instance()
		{	return Singleton<T>::_instance.get();	}

		template<class ...Us>
		static T *Reset(Us &&...vals)
		{
			Singleton<T>::_instance.reset(new T(std::forward<Us>(vals)...));
			return Instance();
		}

		static void Delete()
		{
			Singleton<T>::_instance.reset(nullptr);
		}

	private:
		static std::unique_ptr<T> _instance;

		Singleton() = default;

		friend T;
};

template<class T>
std::unique_ptr<T> Singleton<T>::_instance = nullptr;

#endif // VMC_SINGLETON_H
