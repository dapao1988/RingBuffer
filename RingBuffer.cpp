/*
 * RingBuffer.cpp
 *
 *  Created on: May 14, 2018
 *      Author: cannon
 */
#include "queue.hpp"
#include <thread>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace chrono;

std::mutex mut_;
std::condition_variable cond_;

void fun_generate(Queue<int> *queue)
{
	std::cout<<"fun_generate run"<<std::endl;
	if (!queue)
	{
		return;
	}

	for (int i=0;i<100;i++)
	{
		cout<<"enqueue:"<<i<<endl;
		//std::lock_guard<std::mutex> lk(mut_);
		std::unique_lock<std::mutex> lk(mut_);
		cond_.wait(lk, [queue]{return !queue->full();});
		queue->enqueue(i);
		cout<<"enqueue:"<<i<<"  done"<<endl;
		cond_.notify_one();
	}
	//std::this_thread::sleep_for(std::chrono::seconds(1));
}

//void fun_consume(Queue<int> &queue)
void fun_consume(Queue<int> *queue)
{
	std::cout<<"fun_consume run"<<std::endl;
	if (!queue)
	{
		return;
	}

	bool flag = true;
	do
	{
		std::unique_lock<std::mutex> lk(mut_);
		cond_.wait(lk, [queue]{return !queue->empty();});
		std::shared_ptr< int > i = queue->dequeue();
		cout<<"dequeue:"<<dec<<*(i.get())<<endl<<"use count:"<<dec<<i.use_count()<<endl;
		if(*(i.get()) == 99)
		{
			flag = false;
		}
	} while(flag);
	//std::this_thread::sleep_for(std::chrono::seconds(1));

	cout<<"exit fun_consume!"<<endl;

}

int main(int argn, const char* args[])
{
	Queue<int> queue(16);
	auto start = system_clock::now();
	//std::thread t1(fun_generate, std::ref(queue));
	//std::thread t1(fun_generate, std::ref(queue));
	std::thread t1(fun_generate, &queue);
	std::thread t2(fun_consume, &queue);
	t1.join();
	t2.join();
	auto end   = system_clock::now();
	auto duration = duration_cast<microseconds>(end - start);
	cout <<  "花费了"
	     << double(duration.count()) * microseconds::period::num / microseconds::period::den
	     << "秒" << endl;
	return 0;
}
