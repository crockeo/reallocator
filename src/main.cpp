#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

const size_t TRIAL_COUNT = 100;
const size_t SILLY_DATA_COUNT = 10000000;

template <typename T>
struct ReallocatorNode {
  ReallocatorNode *next;
  T* value;
};

template <typename T>
class ReallocatorQueue {
public:
  ReallocatorQueue() {
    this->head = nullptr;
    this->tail = nullptr;
  }

  void enqueue(T* value) {
    auto next = new ReallocatorNode<T>();
    next->next = nullptr;
    next->value = value;

    if (this->tail != nullptr) {
      this->tail->next = next;
    }
    this->tail = next;
  }

  T* dequeue() {
    if (this->head == nullptr) {
      return nullptr;
    }

    auto head = this->head;
    auto value = head->value;
    this->head = this->head->next;

    delete head;

    return value;
  }

  bool hasNext() {
    return this->head != nullptr;
  }

private:
  ReallocatorNode<T> *head;
  ReallocatorNode<T> *tail;
};

template <typename T>
class Allocator {
public:
  virtual T* alloc() = 0;
  virtual void free(T*) = 0;
};

template <typename T>
class Reallocator : public Allocator<T> {
public:
  Reallocator() {}

  Reallocator(const Reallocator&&) = delete;
  Reallocator operator=(const Reallocator&&) = delete;

  ~Reallocator() {
    T *value;
    while ((value = this->queue.dequeue()) != nullptr) {
      delete value;
    }
  }

  T* alloc() {
    if (!this->queue.hasNext()) {
      return (T *)malloc(sizeof(T));
    }
    return this->queue.dequeue();
  }

  void free(T* t) {
    this->queue.enqueue(t);
  }

private:
  ReallocatorQueue<T> queue;
};

template <typename T>
class NormalAllocator : public Allocator<T> {
public:
  T* alloc() {
    return new T();
  }

  void free(T* value) {
    delete value;
  }
};

template <typename T>
class ArenaAllocator : public Allocator<T> {
public:
  ArenaAllocator(size_t maxCount) {
    this->values.resize(maxCount);
    this->openValues.resize(maxCount);

    for (size_t i = 0; i < maxCount; i++) {
      this->openValues[i] = i;
    }
  }

  T* alloc() {
    if (this->openValues.size() == 0) {
      return nullptr;
    }

    size_t i = openValues.back();
    openValues.pop_back();

    return &values[i];
  }

  void free(T* value) {
    size_t location = (value - &values[0]) / sizeof(T);
    openValues.push_back(location);
  }

private:
  std::vector<T> values;
  std::vector<size_t> openValues;
};

struct SillyData {
  uint64_t pieces[100];
};

enum SimAction {
  ALLOC,
  FREE,
};

void performSim(Allocator<SillyData>& allocator, const std::vector<SimAction>& actions) {
  std::vector<SillyData *> values;
  values.reserve(actions.size());

  SillyData *value;
  for (const SimAction simAction : actions) {
    if (simAction == SimAction::ALLOC) {
      value = allocator.alloc();
      values.push_back(value);
    } else if (simAction == SimAction::FREE && values.size() > 0) {
      value = values.back();
      values.pop_back();

      allocator.free(value);
    }
  }

  for (SillyData *value : values) {
    allocator.free(value);
  }
}

double uniformRand() {
  return (double)rand() / RAND_MAX;
}

int main() {
  srand(time(nullptr));

  ArenaAllocator<SillyData> fastAllocator(SILLY_DATA_COUNT);
  NormalAllocator<SillyData> normalAllocator;

  std::vector<std::chrono::steady_clock::duration> diffs;
  diffs.resize(TRIAL_COUNT);

  std::vector<SimAction> actions;
  actions.resize(SILLY_DATA_COUNT);

  for (size_t i = 0; i < TRIAL_COUNT; i++) {
    for (size_t j = 0; j < SILLY_DATA_COUNT; j++) {
      auto r = uniformRand();
      if (r < 0.5) {
        actions[j] = SimAction::ALLOC;
      } else {
        actions[j] = SimAction::FREE;
      }
    }

    auto fastAllocatorStart = std::chrono::steady_clock::now();
    performSim(fastAllocator, actions);
    auto fastAllocatorStop = std::chrono::steady_clock::now();

    auto normalAllocatorStart = std::chrono::steady_clock::now();
    performSim(normalAllocator, actions);
    auto normalAllocatorStop = std::chrono::steady_clock::now();

    auto fastAllocatorTime = fastAllocatorStop - fastAllocatorStart;
    auto normalAllocatorTime = normalAllocatorStop - normalAllocatorStart;

    auto diff = normalAllocatorTime - fastAllocatorTime;
    diffs[i] = diff;
  }

  std::chrono::steady_clock::duration acc;
  for (const auto diff : diffs) {
    acc += diff;
  }
  acc /= diffs.size();

  std::cout << "average diff " << (acc.count()) / 1000000000.0 << std::endl;
}
