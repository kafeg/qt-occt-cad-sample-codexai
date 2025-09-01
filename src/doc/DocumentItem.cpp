#include "DocumentItem.h"

#include <atomic>
#include <mutex>

namespace {
static std::atomic<DocumentItem::Id>& nextId()
{
  static std::atomic<DocumentItem::Id> v{1};
  return v;
}

static std::unordered_map<DocumentItem::Kind, DocumentItem::CreateFn>& registry()
{
  static std::unordered_map<DocumentItem::Kind, DocumentItem::CreateFn> r;
  return r;
}

static std::mutex registryMutex;
}

DocumentItem::DocumentItem()
  : m_id(nextId().fetch_add(1))
{
}

DocumentItem::DocumentItem(DocumentItem::Id existingId)
  : m_id(existingId)
{
  // keep next id ahead
  DocumentItem::Id cur = nextId().load();
  if (existingId >= cur) {
    nextId().store(existingId + 1);
  }
}

void DocumentItem::registerFactory(DocumentItem::Kind k, DocumentItem::CreateFn fn)
{
  std::lock_guard<std::mutex> lock(registryMutex);
  registry()[k] = std::move(fn);
}

std::shared_ptr<DocumentItem> DocumentItem::create(DocumentItem::Kind k)
{
  std::lock_guard<std::mutex> lock(registryMutex);
  auto& reg = registry();
  auto it = reg.find(k);
  if (it == reg.end()) return nullptr;
  return (it->second)();
}
