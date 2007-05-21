/*!
$Id: subjectmanager.cpp 1142 2006-09-22 21:54:02Z jlisee $
 
    (c) Copyright 2006
	Space Systems Laboratory, University of Maryland, College Park, MD 20742
 
	@file 
        Contains the implementation for the SubjectManager class.
 
	HISTORY
    20-Jul-2006 J Lisee     Imported into ssl::foundation::pattern library.
    25-Feb-2006 J Lisee     Created.
*/

#include "rtsx/foundation/pattern/h/subjectmanager.h"
#include "rtsx/foundation/pattern/h/subject.h"

namespace ssl {
namespace foundation {
namespace pattern {

SubjectManager::SubjectManager() :
    mSubjects(0)
{
}

SubjectManager::~SubjectManager()
{
    // Remove ownership of all subjects so they won't be destroyed.
    removeSubjects();
} 


void SubjectManager::addSubject(Subject *subject)
{
    if(subject != 0)
        mSubjects.push_back(subject);
}


int SubjectManager::countSubjects()
{
    return mSubjects.size();
}


void SubjectManager::removeSubject(Subject *subject)
{
    for(SubjectListIter iter = mSubjects.begin(); 
        iter != mSubjects.end(); iter++)
    {
        // Since the ptr_container takes ownership we have to release the 
        // object.  We then have to release the auto_ptr it returns so the 
        // object won't be deleted when it goes out of scope.
        if(iter->equals(*subject))
        {
            mSubjects.release(iter).release();
            return;
        }
    }
}

void SubjectManager::removeSubjects()
{
    // Same ownership issue described in removeSubject
    while(mSubjects.size() > 0)
        mSubjects.release(mSubjects.begin()).release();
}

void SubjectManager::notifySubjects()
{
    for(size_t i = 0; i < mSubjects.size(); ++i)
        mSubjects[i].notifyObservers();
}

} // namespace pattern
} // namespace foundation
} // namespace ssl
