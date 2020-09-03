#pragma once
#include <stdint.h>
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <arpa/inet.h>
#endif
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <chrono>
#include <random>
#include <functional>
#include <unordered_map>
using namespace std;

namespace kuaishou {
namespace kpbase {

class NonCopyable {
  protected:
    NonCopyable() {}
    virtual ~NonCopyable() {}

  private:
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
    NonCopyable(NonCopyable&&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
};

template <typename T>
class Nullable {
  public:
    Nullable() : is_null_(true) {}
    Nullable(const T& v) : value_(v), is_null_(false) {}
    Nullable(const Nullable& o) : value_(o.value_), is_null_(o.is_null_) {}

    bool IsNull() {
        return is_null_;
    }

    const T& Value() const {
        return value_;
    }

    operator T() const {
        return value_;
    }

  private:
    T value_;
    bool is_null_;
};

class SystemUtil {
  public:
    static uint64_t GetCPUTime();
    static uint64_t GetEpochTime();
    static unordered_map<string, string> GetLocalIPs();
    static string GetDocumentPath();
    static string GetExecFilePath();
    static string GetTimeString(uint64_t ms_epoch, int timezone, bool with_delimiter = true);
    static string GetDayString(uint64_t ms_epoch, int timezone);
    static string GetHourString(uint64_t ms_epoch, int timezone);
    static string GetSystemOSVersion();
    static string GetDeviceModel();
    //static bool EnableCrashDump(void(*log_func)(const char*));

  private:
    SystemUtil();
};

class StringUtil {
  public:
    static string Int2Str(long long val);
    static string Uint2Str(unsigned long long val);
    static Nullable<long long> Str2Int(string str);
    static Nullable<unsigned long long> Str2Uint(string str);
    static string GetHumanReadableString(int64_t val);
    static string ConcealStr(const string& src);
    static string UnConcealStr(const string& src);
    static string Trim(const string& src);
    static bool EndWith(const string& origin, const string& substr);
    static bool StartWith(const string& origin, const string& substr);
    static vector<string> Split(const string& str, const string& pattern);
    static string UrlEncode(const string& str);
    static string HmacSha1(const void* data, size_t data_len, const void* key, size_t key_len);
    static string Replace(string& str, const string& old_value, const string& new_value);

  private:
    StringUtil();
};

class SocketUtil {
  public:
    SocketUtil() = delete;
    SocketUtil(const SocketUtil&) = delete;

    static bool FillInSockaddr(const char* ip, uint16_t port, void* sockaddr, size_t* addrlen);
    static pair<string, uint16_t> GetAddress(const struct sockaddr* addr);
};

template<class T>
class Singleton {
  public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }
};

template<class TimeUnit>
class Profiler {
  public:
    Profiler(uint64_t* val = nullptr, function<void(uint64_t)> finish_func = nullptr) : val_(val),
        finish_func_(finish_func), start_(chrono::steady_clock::now()) {}

    ~Profiler() {
        chrono::steady_clock::duration delta = chrono::steady_clock::now() - start_;
        uint64_t diff = chrono::duration_cast<TimeUnit>(delta).count();
        if (val_)
            *val_ = diff;
        if (finish_func_)
            finish_func_(diff);
    }

    uint64_t GetTime() {
        chrono::steady_clock::duration delta = chrono::steady_clock::now() - start_;
        uint64_t diff = chrono::duration_cast<TimeUnit>(delta).count();
        return diff;
    }

  private:
    uint64_t* val_;
    function<void(uint64_t)> finish_func_;
    chrono::steady_clock::time_point start_;
};

template<typename T>
class ArrayUtil {
  public:
    static void EnsureArraySize(T*& array, size_t& array_size, size_t new_size) {
        if (array_size < new_size) {
            T* new_array = new T[new_size];
            if (array) {
                memcpy(new_array, array, sizeof(T) * array_size);
                delete[] array;
            }
            array = new_array;
            array_size = new_size;
        }
    }
};

template<class T1, class T2, class T3>
struct Trituple {
    T1 first;
    T2 second;
    T3 third;
};

// lack of support for range > int32
template<class IntType = int>
class UniformRandomIntGenerator {
  public:
    explicit UniformRandomIntGenerator(IntType a = 0, IntType b = std::numeric_limits<IntType>::max())
        : dis_(a, b) {
        std::random_device rd;
        gen_.seed(rd());
    }

    ~UniformRandomIntGenerator() {}

    IntType Next() {
        return dis_(gen_);
    }

  private:
    std::mt19937 gen_;
    std::uniform_int_distribution<IntType> dis_;
};

template<class RealType = double>
class UniformRandomRealGenerator {
  public:
    explicit UniformRandomRealGenerator(RealType a = 0, RealType b = std::numeric_limits<RealType>::max())
        : dis_(a, b) {
        std::random_device rd;
        gen_.seed(rd());
    }

    ~UniformRandomRealGenerator() {}

    RealType Next() {
        return dis_(gen_);
    }

  private:
    std::mt19937 gen_;
    std::uniform_real_distribution<RealType> dis_;
};

}
}
