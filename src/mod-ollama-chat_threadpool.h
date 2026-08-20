#ifndef MOD_OLLAMA_CHAT_THREADPOOL_H
#define MOD_OLLAMA_CHAT_THREADPOOL_H

#include <functional>

// Bounded worker pool for the "wait on the LLM response, then route it" tasks used by the direct
// reply, random chatter, and event chatter flows. Replaces one raw std::thread().detach() per
// triggered reply with a fixed-size pool, so a chatter burst can't spawn unbounded OS threads.
// Pool size is OllamaChat.ThreadPoolSize; workers run for the lifetime of the process, matching
// the existing detached-thread lifetime model used elsewhere in this module.
void EnqueueOllamaChatTask(std::function<void()> task);

#endif // MOD_OLLAMA_CHAT_THREADPOOL_H
