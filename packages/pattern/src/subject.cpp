/*!
$Id: subject.cpp 1142 2006-09-22 21:54:02Z jlisee $
 
    (c) Copyright 2006
	Space Systems Laboratory, University of Maryland, College Park, MD 20742
 
	@file 
        Contains the implementation for the Subject class.
 
	HISTORY
    20-Jul-2006 J Lisee     Imported into ssl::foundation::pattern library.
*/

#include "rtsx/foundation/pattern/h/subject.h"
#include "rtsx/foundation/pattern/h/observer.h"

namespace ssl {
namespace foundation {
namespace pattern {

Subject::Subject() :
    mObservers(0)
{
    mChanged = false;
}

Subject::~Subject()
{
    // Remove ownership of all observers so they won't be destroyed.
    removeObservers();
}

void Subject::addObserver(Observer *o)
{
    if(o != 0)
        mObservers.push_back(o);
}


int Subject::countObservers()
{
    return mObservers.size();
}


void Subject::removeObserver(Observer *o)
{
    for(ObserverListIter iter = mObservers.begin(); iter != mObservers.end(); 
        iter++)
    {
        // Since the ptr_container takes ownership we have to release the 
        // object.  We then have to release the auto_ptr it returns so the 
        // object won't be deleted when it goes out of scope.
        if(iter->equals(*o))
        {
            mObservers.release(iter).release();
            return;
        }
    }
}

void Subject::removeObservers()
{
    // Same ownership issue described in removeObserver
    while(mObservers.size() > 0)
        mObservers.release(mObservers.begin()).release();
}

bool Subject::hasChanged()
{
    return mChanged;
}

void Subject::notifyObservers(void* data, long flag)
{
    if(hasChanged())
    {
        for(size_t i = 0; i < mObservers.size(); ++i)
            mObservers[i].update(this, data, flag);
    }
    clearChanged();
}

void Subject::clearChanged()
{
    mChanged = false;
}


void Subject::setChanged()
{
    mChanged = true;
}

} // namespace pattern
} // namespace foundation
} // namespace ssl
