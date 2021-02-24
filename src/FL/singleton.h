#pragma once

#include <memory>

namespace FL {

template <typename T, class X = void, int N = 0>
class Singleton {
  public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
  private:
};

template <class T, class X = void, int N = 0>
class SingletonPtr {
  public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> ptr = std::make_shared<T>();
        return ptr;
    }
};
}
