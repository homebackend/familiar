#ifndef COMMON_H
#define COMMON_H

#include <typeinfo>

#define	WSIMANIP					ws >> noskipws

#define STRINGIZE(nontext)				# nontext

//#define DEBUG
#ifdef DEBUG
 #define DEBUGTHIS(message)		\
	qDebug () << __FUNCTION__ << "::" << __LINE__ << ": " << message;

 #define DEBUGVARIABLEANDITSVALUE(variable)	DEBUGTHIS(STRINGIZE(variable) << variable)

#else

 #define DEBUGTHIS(message)
 #define DEBUGVARIABLEANDITSVALUE(variable)

#endif // DEBUG



#define FIXME "***Fix Me***"

#define GETFUNCTIONSIGNATURE(function,type)		type get ## function () const
#define GETFUNCTIONBODY(argument)			return _ ## argument;
#define	GETFUNCTION(function,type,argument)		GETFUNCTIONSIGNATURE(function,type) { GETFUNCTIONBODY(argument) }

#define	ISFUNCTION(function,argument)			bool is ## function () const { return _ ## argument; }

#define	SETFUNCTIONDECL(function,type,argument)		bool set ## function (const type argument);

#define SETFUNCTIONSIGNATURE(class,function,type,argument) bool class::set ## function (const type argument)
#define SETFUNCTIONBODY(class,type,argument) \
	if (_ ## argument == argument) \
		return false; \
\
	_ ## argument = argument; \
\
	updateModificationTime (); \
	return true;
#define SETFUNCTIONBODYFORQSTRING(class,argument) \
	QString t ## argument = argument.trimmed(); \
	if (_ ## argument == t ## argument) \
		return false; \
\
	_ ## argument = t ## argument; \
\
	updateModificationTime (); \
	return true;
#define	SETFUNCTION(class,function,type,argument)	SETFUNCTIONSIGNATURE(class,function,type,argument) { SETFUNCTIONBODY(class,type,argument) }
#define SETFUNCTIONBODYWITHCONSTCAST(class,type,argument) \
	if (_ ## argument == argument)\
		return false;\
\
	_ ## argument = const_cast<type> (argument); \
\
	updateModificationTime ();\
	return true;
#define	SETFUNCTIONWITHCONSTCAST(class,function,type,argument)	SETFUNCTIONSIGNATURE(class,function,type,argument) { SETFUNCTIONBODYWITHCONSTCAST(class,type,argument) }

#include <QTextStream>
#include <QDebug>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include <QtXml/QDomNodeList>

#endif /* COMMON_H */
