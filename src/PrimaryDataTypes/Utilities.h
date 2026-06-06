#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>
#include <QFontMetrics>
#include <QRect>
#include <QWidget>
#include <QStyleOptionViewItem>

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

    static QRect        getScreenGeometry(QWidget* parentWidget);

    static bool         safeCanConvert(const QVariant &variant, int targetTypeId);

private:
	static void			convertFTMLToDisplayFormatHelperFunction
							(QString &output, QString &endOut,
							 const int &rotateAt, QString &tempOut,
							 QString &plainOutput);
};

#endif // UTILITIES_H
