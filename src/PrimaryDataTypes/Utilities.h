#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QFontMetrics>

class Individual;
class Familiar;

class Utilities
{
public:
	Utilities();

    static Familiar*	stringToFamiliar	(QString input, Individual *individual, QString &tag);

	static QString		convertFTMLToDisplayFormat
							(const QString &input, QString &output,
							 const bool useFullName, const bool createAnchor,
							 const int rotateAt = -1,
							 Individual *individual = NULL);

    static int          horizontalAdvance(QFontMetrics &fmx, const QString &input);

private:
	static void			convertFTMLToDisplayFormatHelperFunction
							(QString &output, QString &endOut,
							 const int &rotateAt, QString &tempOut,
							 QString &plainOutput);
};

#endif // UTILITIES_H
