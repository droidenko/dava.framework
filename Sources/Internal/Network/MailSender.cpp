//
//  MailSender.cpp
//  
//
//  Created by Denis Bespalov on 2/18/13.
//
//

#include "Network/MailSender.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Utils/StringFormat.h"
#include <shellapi.h>

namespace DAVA
{

bool MailSender::SendEmail(const WideString& email, const WideString& subject, const WideString& messageText)
{
	// Don't try to send email if recipient is not set
	if (email.empty() || email == L"")
		return false;
	WideString mailtoText = L"";
	
	// Create mailto text
	mailtoText = Format(L"mailto:?to=%s&subject=%s&body=%s", email.c_str(), subject.c_str(), messageText.c_str());
	// Call default mail client
	int result = (int)ShellExecute(NULL, L"open", mailtoText.c_str(), NULL, NULL, SW_SHOWNORMAL);
	
	// If the function succeeds, it returns a value greater than 32.
	// If the function fails, it returns an error value that indicates the cause of the failure. 
	return (result > 32);
}

};

#elif defined(__DAVAENGINE_ANDROID__)
#include "JniExtensions.h"
#include "Utils/Utils.h"
#include "Utils/UTF8Utils.h"

namespace DAVA
{

class JniMailSender: public JniExtension
{
public:
	JniMailSender();

	bool SendEmail(const String& email, const String& subject, const String& messageText);

public:
	static JniMailSender* jniMailSender;
};

JniMailSender* JniMailSender::jniMailSender = NULL;

JniMailSender::JniMailSender() :
	JniExtension("com/dava/framework/JNISendMail")
{

}

bool JniMailSender::SendEmail(const String& email, const String& subject, const String& messageText)
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return false;

	jmethodID mid = GetMethodID(javaClass, "SendEMail", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
	bool res = false;
	if (mid)
	{
		jstring jEMail = GetEnvironment()->NewStringUTF(email.c_str());
		jstring jSubject = GetEnvironment()->NewStringUTF(subject.c_str());
		jstring jMessageText = GetEnvironment()->NewStringUTF(messageText.c_str());
		res = GetEnvironment()->CallStaticBooleanMethod(javaClass, mid, jEMail, jSubject, jMessageText);
		GetEnvironment()->DeleteLocalRef(jEMail);
		GetEnvironment()->DeleteLocalRef(jSubject);
		GetEnvironment()->DeleteLocalRef(jMessageText);
	}
	ReleaseJavaClass(javaClass);

	return res;
}

bool MailSender::SendEmail(const WideString& email, const WideString& subject, const WideString& messageText)
{
	if (!JniMailSender::jniMailSender)
		JniMailSender::jniMailSender = new JniMailSender();

	return JniMailSender::jniMailSender->SendEmail(UTF8Utils::EncodeToUTF8(email), UTF8Utils::EncodeToUTF8(subject), UTF8Utils::EncodeToUTF8(messageText));
}

};

#endif
