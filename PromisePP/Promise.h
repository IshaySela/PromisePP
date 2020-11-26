#pragma once
#include <functional>
#include <iostream>
#include <future>
#include <map>
#include <mutex>

using std::function;

namespace PromisePP
{
	
	namespace {
		// Store all of the pending futures.
		std::map<int, std::shared_future<void>> pendingMap;
		std::mutex mapLock;
		// Return a future id.
		int getCounter() 
		{
			static int counter = 0;
			return ++counter;
		}
		std::shared_future<void> getFuture(int id)
		{
			std::lock_guard<std::mutex> guard(mapLock);
			return pendingMap[id];
		}
		
	}

	template <typename TResolve, typename TReject>
	class Promise
	{
	public:
		Promise(function<void(function<void(TResolve)>, function<void(TReject)>)> executor);
		~Promise();

		/**
		 * @brief Handle the result of the resolve() call in the executor.
		 * 
		 * @param handler Function that handles the result of the executor.
		 * @tparam TResult The type of the return value of the handler.
		 * @return A new promise made out of the handler.
		 */
		template <typename TResult>
		Promise<TResult, std::exception> then(function<TResult(TResolve)> handler);
		/**
		 * @brief Handle the result of the reject() call in the executor.
		 * 
		 * @param handler The function that handles the reject.
		 * @tparam TResult The type of the return value of the handler.
		 * @return A new promise made out of the handler.
		 */
		template <typename TResult>
		Promise<TResult, std::exception> error(function<TResult(TReject)> handler);

		/**
		 * @brief return the shared future of this object.
		 * 
		 * @return A std::shared_future that is waiting for the execution of this Promise. 
		 */
		std::shared_future<void> getFuture() const 
		{ 
			auto x = pendingMap[this->futureId];
			return x;
		}
	private:
		TResolve resolveValue;
		TReject rejectValue;

		bool resolveCalled = false, rejectCalled = false;

		// Store the id of the future.
		int futureId = 0;
	};
	
	
	template<typename TResolve, typename TReject>
	inline Promise<TResolve, TReject>::Promise(function<void(function<void(TResolve)>, function<void(TReject)>)> executor)
	{
		auto resolve = [this](TResolve value)
		{
			this->resolveValue = value;
			this->resolveCalled = true;
		};
		
		auto reject = [this](TReject value)
		{	
			this->rejectValue = value;
			this->rejectCalled = true;
		};

		this->futureId = getCounter();
		pendingMap.insert(std::pair<int, std::future<void>>(this->futureId, std::async(std::launch::async, executor, resolve, reject)));
	}


	template<typename TResolve, typename TReject>
	Promise<TResolve, TReject>::~Promise()
	{
		try
		{
			// Wait for the future to finish and then remove it.
			pendingMap[futureId].wait();
			pendingMap.erase(futureId);
		}
		catch (...) { }
	}

	template <typename TResolve, typename TReject>
	template <typename TResult>
	inline Promise<TResult, std::exception> Promise<TResolve, TReject>::error(function<TResult(TReject)> handler)
	{
		return Promise<TResult, std::exception>(
			[=](std::function<void(TResult)> resolve, std::function<void(std::exception)> reject)
			{
				pendingMap[this->futureId].wait();

				if (!this->rejectCalled) return;

				try
				{
					handler(this->rejectValue);
				}
				catch (std::exception e)
				{
					reject(e);
				}

			}
		);
	}
	
	template<typename TResolve, typename TReject>
	template<typename TResult>
	inline Promise<TResult, std::exception> Promise<TResolve, TReject>::then(function<TResult(TResolve)> handler)
	{
		auto exe = [this, handler](std::function<void(TResult)> resolve, std::function<void(std::exception)> reject)
		{
			auto ftr = pendingMap[this->futureId];
			auto d = ftr.valid();
			if (ftr.valid()) ftr.wait();

			// Handler wont be called in resolve() wasnt called either.
			if (!this->resolveCalled) return;

			try
			{
				resolve(handler(this->resolveValue));
			}
			catch (std::exception e)
			{
				reject(e);
			}

		};


		return Promise<TResult, std::exception>(exe);
	}

	
}