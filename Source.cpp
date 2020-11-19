#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>
#include <string>
#include <condition_variable>
using namespace std;
bool finish = false;	//флаг остановки экзамена
mutex serverready, exchange1, exchange2, serveranswer;	//мьютексы для синхронизации
bool bserverready=false, bexchange1 = false, bexchange2 = false, bserveranswer = false;	//флаги событий для синхронизации
condition_variable cserverready, cexchange1, cexchange2, cserveranswer;	//условные переменные
mutex report;	//мьютекс для вывода на экран
struct
{
	int stud;	//номер студента
	union 
	{
		int ticket;	//номер билета
		int mark;	//оценка
	}x;
}shared;	//область для обмена данными между клиентом и сервером
//вывод сообщения
void showmessage(stringstream &msg)
{
	report.lock();
	cout << msg.str();
	msg.str("");
	report.unlock();
}
//сервер
void teacher()
{
	std::unique_lock<std::mutex> lexchange1(exchange1);
	std::unique_lock<std::mutex> lexchange2(exchange2);

	stringstream ss;
	while (!finish)	//ПРодолжаем пока не закончится экзамен
	{
		//установить событие что сервер(преподаватель) свободен
		bserverready = true;
		cserverready.notify_one();
		//Ожидаем пока кто-то из ожидающих клиентов заполнит общую область памяти
		while (!bexchange1) {  // цикл чтобы избежать случайного пробуждения
			cexchange1.wait(lexchange1);
		}
		bexchange1 = false;	//сбросить флаг события
		ss << "Teacher start exam student " << shared.stud << " with ticket " << shared.x.ticket << endl;
		showmessage(ss);
		//преподаватель принимает у студента рандомное время
		this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 10000));
		shared.x.mark = rand() % 3 + 3;	//выставляет оценку за экзамен
		ss << "Teacher marked student " << shared.stud << " with " << shared.x.mark << endl;
		showmessage(ss);
		//Устанавливает событие, что сервер ответил (экзамен сдан)
		bserveranswer = true;
		cserveranswer.notify_one();
		//ожидает пока клиент не подаст сигнал, что он прочитал переданные данные
		while (!bexchange2) {  // цикл чтобы избежать случайного пробуждения
			cexchange2.wait(lexchange2);
		}
		bexchange2 = false;		//сбросить флаг события
		this_thread::sleep_for(chrono::milliseconds(10));	//небольшая задержка, прподавателю нужен отдых)))
	}
}
///клиент
void stud(int ticket, int n)
{
	std::unique_lock<std::mutex> lserverready(serverready);
	std::unique_lock<std::mutex> lserveranswer(serveranswer);

	srand(time(nullptr));
	stringstream ss;
	ss << "Student " << n << " has ticket " << ticket << " and wait" << endl;
	showmessage(ss);

	//Студент готовится рандомное время
	this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 10000));
	//Когда готов, ожидает готовности сервера (преподавателя)
	while (!bserverready) {  // цикл чтобы избежать случайного пробуждения
		cserverready.wait(lserverready);
	}
	bserverready = false;		//сбросить флаг события
	//если готов, садится отвечать
	ss << "Student " << n << " sit to answer ticket " << ticket << endl;
	showmessage(ss);
	//говорит серверу кто это и какой у него билет
	shared.stud = n;
	shared.x.ticket = ticket;
	//подает событие, что данные готовы
	bexchange1 = true;
	cexchange1.notify_one();
	//ожидает когда сервер даст ответ
	while (!bserveranswer) {  // цикл чтобы избежать случайного пробуждения
		cserveranswer.wait(lserveranswer);
	}
	bserveranswer = false;		//сбросить флаг события
	ss << "Student " << shared.stud << " marked for " << shared.x.mark << endl;
	showmessage(ss);
	//Подает сигнал серверу, что область общей памяти свободна и можно принимать следующего
	bexchange2 = true;
	cexchange2.notify_one();
	ss << "Student " << n << " leave room" << endl;
	showmessage(ss);

}
int main()
{
	int n;
	//Создать поток-сервер
	thread t(teacher);
	cout << "Number of students: ";
	cin >> n;
	if (n <= 0) {
		cout << "incorrect input data";
		return 1;
	}
	vector<thread> students;
	//Создать потоки студентов
	for (int i = 0; i < n; i++)
	{
		students.push_back(thread(stud, rand() % n, i + 1));
	}
	//ожидать их завершения
	for (int i = 0; i < n; i++)
	{
		students[i].join();
	}
	finish = true;	//закончить экзамен
	t.join();
	system("pause");
	return 0;
}