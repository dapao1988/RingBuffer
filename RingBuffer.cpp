/*
 * RingBuffer.cpp
 *
 *  Created on: May 14, 2018
 *      Author: cannon
 */
#include "queue.hpp"
#include <thread>
#include <iostream>

using namespace std;

void fun_generate(Queue<int> &queue)
{
	std::cout<<"fun_generate run"<<std::endl;
	for (int i=0;i<100;i++)
	{
		cout<<"enqueue:"<<i<<endl;
		queue.enqueue(i);
		cout<<"enqueue:"<<i<<"done"<<endl;
	}
}

void fun_consume(Queue<int> &queue)
{
	std::cout<<"fun_consume run"<<std::endl;
	do
	{
		while(queue.empty() == 0)
		{
			std::shared_ptr< int > i = queue.dequeue();
			cout<<"dequeue:"<<dec<<*(i.get())<<endl<<"use count:"<<dec<<i.use_count()<<endl;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	} while (true);
	cout<<"exit fun_consume!"<<endl;
}

int main(int argn, const char* args[])
{
	Queue<int> queue(16);
	std::thread t1(fun_generate, std::ref(queue));
	std::thread t2(fun_consume, std::ref(queue));
	t1.join();
	return 0;
}
