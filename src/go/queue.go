package mrpc

import (
	"sync"
)

type call struct {
	Finish bool
	Result string
	err    error
	done   chan struct{}
}

func newCall() *call {
	return &call{
		done: make(chan struct{}),
	}
}

func (c *call) Set(finish bool, result string, err error) {
	c.Finish = finish
	c.Result = result
	c.err = err
	close(c.done)
}

func (c *call) Wait() {
	<-c.done
}

type clientQueue struct {
	mu    sync.Mutex
	queue map[string]*call
}

func newClientQueue() *clientQueue {
	return &clientQueue{
		queue: make(map[string]*call),
	}
}

func (q *clientQueue) WaitResult(key string) {
	q.mu.Lock()
	defer q.mu.Unlock()
	q.queue[key] = newCall()
}

func (q *clientQueue) SetResult(key string, result string, err error) {
	q.mu.Lock()
	defer q.mu.Unlock()
	call, ok := q.queue[key]
	if !ok {
		panic("key not found in queue")
	}
	call.Set(true, result, err)
}

func (q *clientQueue) GetResult(key string) (string, error) {
	q.mu.Lock()
	call, ok := q.queue[key]
	if !ok {
		panic("key not found in queue")
	}
	q.mu.Unlock()

	call.Wait()
	result := call.Result
	err := call.err

	q.mu.Lock()
	delete(q.queue, key)
	q.mu.Unlock()

	return result, err
}
