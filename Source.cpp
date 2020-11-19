#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <sstream>
#include <string>
#include <condition_variable>
using namespace std;
bool finish = false;	//���� ��������� ��������
mutex serverready, exchange1, exchange2, serveranswer;	//�������� ��� �������������
bool bserverready=false, bexchange1 = false, bexchange2 = false, bserveranswer = false;	//����� ������� ��� �������������
condition_variable cserverready, cexchange1, cexchange2, cserveranswer;	//�������� ����������
mutex report;	//������� ��� ������ �� �����
struct
{
	int stud;	//����� ��������
	union 
	{
		int ticket;	//����� ������
		int mark;	//������
	}x;
}shared;	//������� ��� ������ ������� ����� �������� � ��������
//����� ���������
void showmessage(stringstream &msg)
{
	report.lock();
	cout << msg.str();
	msg.str("");
	report.unlock();
}
//������
void teacher()
{
	std::unique_lock<std::mutex> lexchange1(exchange1);
	std::unique_lock<std::mutex> lexchange2(exchange2);

	stringstream ss;
	while (!finish)	//���������� ���� �� ���������� �������
	{
		//���������� ������� ��� ������(�������������) ��������
		bserverready = true;
		cserverready.notify_one();
		//������� ���� ���-�� �� ��������� �������� �������� ����� ������� ������
		while (!bexchange1) {  // ���� ����� �������� ���������� �����������
			cexchange1.wait(lexchange1);
		}
		bexchange1 = false;	//�������� ���� �������
		ss << "Teacher start exam student " << shared.stud << " with ticket " << shared.x.ticket << endl;
		showmessage(ss);
		//������������� ��������� � �������� ��������� �����
		this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 10000));
		shared.x.mark = rand() % 3 + 3;	//���������� ������ �� �������
		ss << "Teacher marked student " << shared.stud << " with " << shared.x.mark << endl;
		showmessage(ss);
		//������������� �������, ��� ������ ������� (������� ����)
		bserveranswer = true;
		cserveranswer.notify_one();
		//������� ���� ������ �� ������ ������, ��� �� �������� ���������� ������
		while (!bexchange2) {  // ���� ����� �������� ���������� �����������
			cexchange2.wait(lexchange2);
		}
		bexchange2 = false;		//�������� ���� �������
		this_thread::sleep_for(chrono::milliseconds(10));	//��������� ��������, ������������ ����� �����)))
	}
}
///������
void stud(int ticket, int n)
{
	std::unique_lock<std::mutex> lserverready(serverready);
	std::unique_lock<std::mutex> lserveranswer(serveranswer);

	srand(time(nullptr));
	stringstream ss;
	ss << "Student " << n << " has ticket " << ticket << " and wait" << endl;
	showmessage(ss);

	//������� ��������� ��������� �����
	this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 10000));
	//����� �����, ������� ���������� ������� (�������������)
	while (!bserverready) {  // ���� ����� �������� ���������� �����������
		cserverready.wait(lserverready);
	}
	bserverready = false;		//�������� ���� �������
	//���� �����, ������� ��������
	ss << "Student " << n << " sit to answer ticket " << ticket << endl;
	showmessage(ss);
	//������� ������� ��� ��� � ����� � ���� �����
	shared.stud = n;
	shared.x.ticket = ticket;
	//������ �������, ��� ������ ������
	bexchange1 = true;
	cexchange1.notify_one();
	//������� ����� ������ ���� �����
	while (!bserveranswer) {  // ���� ����� �������� ���������� �����������
		cserveranswer.wait(lserveranswer);
	}
	bserveranswer = false;		//�������� ���� �������
	ss << "Student " << shared.stud << " marked for " << shared.x.mark << endl;
	showmessage(ss);
	//������ ������ �������, ��� ������� ����� ������ �������� � ����� ��������� ����������
	bexchange2 = true;
	cexchange2.notify_one();
	ss << "Student " << n << " leave room" << endl;
	showmessage(ss);

}
int main()
{
	int n;
	//������� �����-������
	thread t(teacher);
	cout << "Number of students: ";
	cin >> n;
	if (n <= 0) {
		cout << "incorrect input data";
		return 1;
	}
	vector<thread> students;
	//������� ������ ���������
	for (int i = 0; i < n; i++)
	{
		students.push_back(thread(stud, rand() % n, i + 1));
	}
	//������� �� ����������
	for (int i = 0; i < n; i++)
	{
		students[i].join();
	}
	finish = true;	//��������� �������
	t.join();
	system("pause");
	return 0;
}