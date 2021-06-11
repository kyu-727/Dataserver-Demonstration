#include "pcbuffer.hpp"

using namespace std;

PCBuffer::PCBuffer(int _size)
{
    size = _size;
    emp = Semaphore(size);
}
PCBuffer::~PCBuffer() {}
int PCBuffer::Deposit(string _item)
{
    // cout << "deposit " << _item << endl;
    emp.P();
    lock.P();
    buffQ.push(_item);
    lock.V();
    full.V();
    return 0;
}

string PCBuffer::Retrieve()
{
    full.P();
    lock.P();
    string poppedString = buffQ.front();
    //cout << "retrieved " << poppedString << endl;
    buffQ.pop();
    lock.V();
    emp.V();
    return poppedString;
}