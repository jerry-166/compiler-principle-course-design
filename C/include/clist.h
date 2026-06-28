#ifndef CLIST_H
#define CLIST_H

// MFC CList 的最小替代实现（纯标准 C++）。
// 原始 PL/0 源码（2003 年 VC/TC）用 SYMLIST 继承自 MFC 的 CList<SYMBOL,SYMBOL>。
// Linux 下无 MFC，这里实现一个只覆盖源码实际用到的接口的最小链表模板，
// 使 PL.cpp / common.h 无需改动即可在 GCC 下编译。

// 源码实际用到的 CList 接口（已逐一核对 PL.cpp）：
//   AddHead(T)                  头插单个元素
//   AddTail(T)                  尾插单个元素
//   AddTail(const CList*)       把另一个链表整体追加到尾部（批量）
//   GetHeadPosition() -> POSITION   取头游标
//   GetNext(POSITION&) -> T         顺次取出，游标后移，末尾置 NULL
// POSITION 用内部节点指针表示，空链表/到末尾时为 NULL（与 MFC 行为一致）。

template <class TYPE, class ARG_TYPE>
class CList
{
    struct Node
    {
        TYPE data;
        Node* next;
        Node* prev;
        explicit Node(const ARG_TYPE& v) : data(v), next(NULL), prev(NULL) {}
    };

    Node* m_pHead;
    Node* m_pTail;

public:
    CList() : m_pHead(NULL), m_pTail(NULL) {}

    ~CList()
    {
        Node* p = m_pHead;
        while (p)
        {
            Node* n = p->next;
            delete p;
            p = n;
        }
    }

    // 头插
    void AddHead(ARG_TYPE newElement)
    {
        Node* p = new Node(newElement);
        p->next = m_pHead;
        if (m_pHead)
            m_pHead->prev = p;
        else
            m_pTail = p;
        m_pHead = p;
    }

    // 尾插单个
    void AddTail(ARG_TYPE newElement)
    {
        Node* p = new Node(newElement);
        p->prev = m_pTail;
        if (m_pTail)
            m_pTail->next = p;
        else
            m_pHead = p;
        m_pTail = p;
    }

    // 把另一个链表整体追加到尾部（MFC CList::AddTail 的批量重载）
    void AddTail(const CList<TYPE, ARG_TYPE>* pNewList)
    {
        if (!pNewList)
            return;
        POSITION pos = pNewList->GetHeadPosition();
        while (pos)
        {
            TYPE t = pNewList->GetNext(pos);
            AddTail(t);
        }
    }

    // POSITION 就是节点指针；遍历到末尾时 GetNext 会把它置为 NULL。
    typedef Node* POSITION;

    POSITION GetHeadPosition() const { return m_pHead; }

    TYPE GetNext(POSITION& rPosition) const
    {
        Node* p = rPosition;
        rPosition = p->next;
        return p->data;
    }
};

#endif
