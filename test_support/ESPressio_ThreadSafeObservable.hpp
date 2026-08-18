#pragma once
#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>
#include "ESPressio_IObserver.hpp"
namespace ESPressio { namespace Observable {
class IObservable { public: virtual ~IObservable() = default; };
class IObserverHandle { public: virtual ~IObserverHandle()=default; virtual void Unregister()=0; virtual IObserver* GetObserver()=0; };
using ObserverHandlePtr=std::unique_ptr<IObserverHandle>;
class ThreadSafeObservable;
class ObserverHandle final: public IObserverHandle {
    ThreadSafeObservable* owner_; IObserver* observer_;
public:
    ObserverHandle(ThreadSafeObservable* o,IObserver* i):owner_(o),observer_(i){}
    ~ObserverHandle() override { Unregister(); }
    void Unregister() override;
    IObserver* GetObserver() override { return observer_; }
    void Invalidate(){owner_=nullptr;observer_=nullptr;}
};
class ThreadSafeObservable: public IObservable, public std::enable_shared_from_this<ThreadSafeObservable> {
    std::vector<ObserverHandle*> observers_; std::recursive_mutex mutex_;
protected:
    class NotificationContext {
        ThreadSafeObservable& o_;
    public:
        explicit NotificationContext(ThreadSafeObservable& o):o_(o){}
        template<class TObserver,class Callback> void WithObservers(Callback&& cb){
            std::lock_guard<std::recursive_mutex> l(o_.mutex_);
            auto copy=o_.observers_;
            for(auto* h:copy){ if(!h) continue; auto* x=dynamic_cast<TObserver*>(h->GetObserver()); if(x) cb(x); }
        }
    };
    template<class Operation> void ExecuteNotification(Operation&& op){ auto keep=shared_from_this(); NotificationContext c(*this); op(c); }
public:
    ~ThreadSafeObservable() override { std::lock_guard<std::recursive_mutex> l(mutex_); for(auto* h:observers_) if(h) h->Invalidate(); observers_.clear(); }
    ObserverHandlePtr RegisterObserver(IObserver* o){ if(!o) throw std::runtime_error("null observer"); std::lock_guard<std::recursive_mutex> l(mutex_); auto* h=new ObserverHandle(this,o); observers_.push_back(h); return ObserverHandlePtr(h); }
    void UnregisterObserver(IObserver* o){ std::lock_guard<std::recursive_mutex> l(mutex_); for(auto it=observers_.begin();it!=observers_.end();++it){ if((*it)->GetObserver()==o){ (*it)->Invalidate(); observers_.erase(it); return; }} }
    void UnregisterHandle(ObserverHandle* h){ std::lock_guard<std::recursive_mutex> l(mutex_); auto it=std::find(observers_.begin(),observers_.end(),h); if(it!=observers_.end()) observers_.erase(it); h->Invalidate(); }
};
inline void ObserverHandle::Unregister(){ if(owner_) owner_->UnregisterHandle(this); }
}}
