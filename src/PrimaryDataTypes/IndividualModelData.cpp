#include "Individual.h"
#include "Family.h"
#include "SettingsManager.h"
#include "Utilities.h"

Qt::ItemFlags Individual::flags(const int col, const bool isSpouseItem) const
{
	Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;

	switch (col)
	{
	case e_col_link:
		break;
	case e_col_sex:
	case e_col_male:
	case e_col_female:
		break;
	case e_col_dead:
		itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_birthDate:
		itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_deathDate:
		if (isDead())
			itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_marriageDate:
		if (isFemale() && isSpouseItem)
			itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_facebookID:
		itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_dummyOrBase:
		itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_myGeneration:
		break;
	default:
#ifdef DEBUGMODE
		qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
        itemFlags = Familiar::flags(col, isSpouseItem);
		break;
	}
	return itemFlags;
}

QVariant Individual::data(const int col, const int role, const IDTYPE spouseID)
{
	if (role == Qt::ForegroundRole)
	{
		switch (col)
		{
		case e_col_name:
			if (getAdoptedTo ())
				return QColor ("Gray");

			if (isFemale())
			{
				if (spouseID)
					return QColor ("blue");
				else
					return QColor ("red");
			}
			else
				return QVariant (QColor ("green"));
		case e_col_birthDate: return QVariant (QColor ("#c0ea00"));
		case e_col_deathDate: return QVariant (QColor ("yellow"));
		case e_col_marriageDate: return QVariant (QColor ("orange"));
		case e_col_link: return QVariant (QColor ("white"));
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	}
	else if (role == Qt::FontRole)
	{
		QFont font;
		switch (col)
		{
		case e_col_name:
			if (isFemale())
			{
				if (spouseID)
					font.setItalic(true);
			}
			else
			{
				font.setBold(true);
			}
			font.setCapitalization(QFont::SmallCaps);
			break;
		case e_col_birthDate:
			{
				Event *event = getBirthEvent();
				if (event && !event->wasDateParsed())
					font.setItalic(true);
				font.setPixelSize(11);
			}
			break;
		case e_col_deathDate:
			{
				Event *event = getDeathEvent();
				if (event && !event->wasDateParsed())
					font.setItalic(true);
				font.setPixelSize(11);
			}
			break;
		case e_col_marriageDate:
			//font.setBold(true);
			if (isFemale() && spouseID)
			{
				Event *event = getMarriageEvent (spouseID);
				if (event && !event->wasDateParsed())
					font.setItalic(true);
			}
			font.setPixelSize(11);
			break;
		case e_col_link:
			font.setItalic(true);
			font.setPixelSize(11);
			break;
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			if (isNonDummy ())
                return Familiar::data(col, role, spouseID);
			break;
		}
		if (isDummy())
			font.setPixelSize(3);
		return font;
	}
	else if (role == Qt::TextAlignmentRole)
	{
		switch (col)
		{
		case e_col_sex: return QVariant(Qt::AlignHCenter);
		case e_col_dead: return QVariant(Qt::AlignHCenter);
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	} /// Qt::TextAlignmentRole

	else if (role == Qt::DecorationRole)
	{
		if (isDummy())
			return QVariant ();

		if (col == e_col_icon)
			return *(getIcon ());

		QString decorationPath = getDecoration (col, spouseID);
		if (decorationPath.isEmpty ())
			return QVariant ();

		return QPixmap (decorationPath);
	} /// Qt::DecorationRole

	else if (role == Qt::ToolTipRole)
	{
		switch (col)
		{
		case e_col_name:
			if (isDummy())
				return "This is a dummy individual. In most cases "
						"it is female spouse item whose information "
						"is unknown. Please update information for this "
						"individual to see more data here.";
			return getToolTip(false, spouseID ? spouseID : 0);
		case e_col_sex:
			if (isFemale())
				return QVariant(QString("%1 is Female").arg(getName()));
			else
				return QString("%1 is Male").arg(getName());
		case e_col_dead:
			if (isDead())
				return QVariant(QString("%1 is no longer with us").arg(getName()));
			else
				return QVariant(QString("%1 is very much with us").arg(getName()));
		case e_col_birthDate:
			if (getBirthEvent())
			{
				if (isDead())
				{
					if(getDeathEvent())
						return QVariant(QString("Value within brackets is the age of %1 at the time of %2 death.").arg(getName()).arg(genderBias (hisher, false)));
					else
						return QVariant(QString("Value within brackets is the years since birth of %1.").arg(getName()));
				}
				else
					return QVariant(QString("Value within brackets is the current age of %1.").arg(getName()));
			}
			return QVariant();
		case e_col_marriageDate:
			if (isFemale() && spouseID)
			{
				if (getMarriageEvent(spouseID))
				{
					Individual *spouse = Individual::s_IdToIndividualHash.value(spouseID);
					if (isDead())
					{
						if(getDeathEvent())
							return QVariant(QString("Value within brackets is the number of years %1 was married with.%2").arg(getName()).arg(spouse->getName()));
						else
							return QVariant(QString("Value within brackets is the years since marriage of %1 with %2.").arg(getName()).arg(spouse->getName()));
					}
					else
						return QVariant(QString("Value within brackets is the number of years since marriage of %1 with %2.").arg(getName()).arg(spouse->getName()));
				}
			}
			return QVariant();
		case e_col_deathDate:
			if (isDead() && getDeathEvent())
				return QVariant(QString("Value within brackets is the years since death of %1.").arg(getName()));
			return QVariant();
		case e_col_icon:
			return data (e_col_name, role, spouseID);
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	} /// Qt::ToolTipRole

	else if (role == Qt::WhatsThisRole)
	{
		QString dateWhatsThis = "<br><br>Enter the date as <b>YYYYMMDD</b> (e.g. <i>20100117</i>)."
					"<br>In case of circa dates enter as <b>c. YYYY</b> or <b>c. YYYYMM</b> (e.g. <i>c. 2010</i> or <i>c. 201001</i>)."
					"<br>Any other string will be treated as plain string and will be stored as is (e.g. <i>Somedate</i>).";

		switch (col)
		{
		case e_col_name:
			return QVariant(QString("<b>Full Name</b> of the individual including the family name."
						"<br>Images are used to depict certain things. A description follows:"
						"<br><img src=':images/son_unmarried.png'> depicts a male not having a spouse"
						"<br><img src=':images/son_married.png'> depicts a male having spouses, but not having a child"
						"<br><img src=':images/son_with_children.png'> depicts a male having children"
						"<br><img src=':images/daughter_unmarried.png'> depicts a female not having a spouse, in the role of a daughter"
						"<br><img src=':images/daughter_married.png'> depicts a female having spouses, in the role of a daughter"
						"<br><img src=':images/wife.png'> depicts a female, currently in the role of a wife"
						"<br><br>This field can be edited inline. However, family name can not be edited inline."));
		case e_col_dead:
			return QVariant(QString("Shows if the individual is dead or alive."
						"<br><img src=':images/dead.png'> means the individual is dead."
						"<br><img src=':images/alive.png'> means the individual is alive."
						"<br><br>This field can be edited inline."));
		case e_col_sex:
			return QVariant(QString("<b>Sex</b> of the individual."
						"<br><img src=':images/male.png'> depicts a male."
						"<br><img src=':images/female.png'> depicts a female."
						"<br><br>This field can not be edited inline."
						"<br>Note changing sex of an individual could potentially lead to data loss."));
		case e_col_birthDate:
			return QVariant(QString("<b>Birthday</b> of the individual."
						"<br>For more information about value in brackets see tooltip."
						"<br><br>This field can be edited inline."
						"%1"
						).arg(dateWhatsThis));
		case e_col_marriageDate:
			return QVariant(QString("The date on which the individual was married."
						"<br>For more information about value in brackets see tooltip."
						"<br><br>This field can be edited inline only in the case of a married female currently in the role of a wife."
						"%1"
						).arg(dateWhatsThis));
		case e_col_deathDate:
			return QVariant(QString("The date on which the individual died."
						"<br>For more information about value in brackets see tooltip."
						"<br><br>This field can be edited inline only if the individual is dead."
						"%1"
						).arg(dateWhatsThis));
		case e_col_link:
			return QVariant(QString("Link to another location on this tree where the individual is present. This field only applies to married females. Click on the text to follow the individual's other locations on the tree."
						"<br><br>This field can not be edited inline."));
		case e_col_icon:
			return "Image of " + getFullName ();
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	} /// Qt::WhatsThisRole

	else if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (col)
		{
		case e_col_name:
			if (role == Qt::DisplayRole)
			{
				if (isDummy())
					return "***";
				else
					return getFullName();
			}
			break;
		case e_col_sex:
			if (role == Qt::EditRole)
				return QVariant (isFemale ());
			return QVariant ();
		case e_col_male:
			if (role == Qt::EditRole)
				return QVariant ();
			return isMale ();
		case e_col_female:
			if (role == Qt::EditRole)
				return QVariant ();
			return isFemale ();
		case e_col_dead:
			if (role == Qt::EditRole)
				return QVariant (isDead());
			return QVariant ();
		case e_col_birthDate:
			{
				Event *birth = getBirthEvent();
				if (birth)
				{
					if (role == Qt::DisplayRole)
					{
						Event *death = getDeathEvent();
						if (death)
							return birth->toDisplayDate (death);
						Dated tDate(QDate::currentDate());
						return birth->toDisplayDate(tDate);
					}
					else
						return birth->getDated().getDateString();
				}
			}
			return QVariant();
		case e_col_deathDate:
			{
				Event *death = getDeathEvent();
				if (death)
				{
					if (role == Qt::DisplayRole)
					{
						Dated tDate(QDate::currentDate());
						return death->toDisplayDate(tDate);
					}
					else
						return death->getDated().getDateString();
				}
			}
			return QVariant();
		case e_col_marriageDate:
			if (isFemale() && spouseID)
			{
				Event *marriage = getMarriageEvent(spouseID);
				if (marriage)
				{
					if (role == Qt::DisplayRole)
					{
						Event *death = getDeathEvent();
						Event *spouseDeath = getSpouseWithID (spouseID)->getDeathEvent ();
						if (death && !spouseDeath)
							return marriage->toDisplayDate(death);
						else if (!death && spouseDeath)
							return marriage->toDisplayDate (spouseDeath);
						else if (death && spouseDeath)
						{
							if (*death < *spouseDeath)
								return marriage->toDisplayDate (death);
							else
								return marriage->toDisplayDate (spouseDeath);
						}
						Dated tDate(QDate::currentDate());
						return marriage->toDisplayDate(tDate);
					}
					else
						return marriage->getDated().getDateString();
				}
			}
			return QVariant();
		case e_col_link:
			if (isFemale())
			{
				if (spouseID)
				{
					if (Individual *father = getFather())
						return QString("From: %1").arg(father->getFullName());
				}
				else if (hasSpouse())
				{
					QString spouses = getSpouse(0)->getFullName();
					for (int i = 1; i < spouseCount(); i++)
						spouses += ", " + getSpouse(i)->getFullName();
					return QString("To: %1").arg(spouses);
				}
			}
			return QVariant();
		case e_col_facebookID:
			if (role == Qt::EditRole)
				return getFacebookID ();
			else
				return getFacebookURL ();
		case e_col_facebookURL:
			return getFacebookURL ();
		case e_col_dummyOrBase:
			if (role == Qt::DisplayRole)
			{
				if (isDummy ())
					return "Is Dummy";
				else
					return "Not Dummy";
			}
			return isDummy();
		case e_col_isIndividual:
			return true;
		case e_col_individualIsDummy:
			return isDummy ();
		case e_col_myGeneration:
            if (getFamily ())
				return getFamily ()->getGenerations() - _generations;
            return 0;
		case e_col_icon:
			if (hasImage ())
				return *(getIcon ());
			return QVariant ();
		case e_col_hasBirthEvent:
			return getBirthEvent () != NULL;
		case e_col_hasDeathEvent:
			return getDeathEvent () != NULL;
		case e_col_hasMarriageEvent:
			return getMarriageEvent (spouseID) != NULL;
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	} /// Qt::DisplayRole || Qt::EditRole

	else if (role == Qt::StatusTipRole)
	{
		switch (col)
		{
		case e_col_sex: return QVariant(QString("Shows the sex"));
		default:
#ifdef DEBUGMODE
			qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
			break;
		}
	}		/// Qt::StatusTipRole

    return Familiar::data(col, role, spouseID);
}

bool Individual::setData(const int col, const QVariant &value, const IDTYPE spouseID)
{
	switch (col)
	{
	case e_col_dead: return setDead(value.toBool());
	case e_col_birthDate:
	{
		QDate date = value.toDate();

		Event *event = getBirthEvent ();
		if (!event)
		{
			event = new Event;
			event->setName ("Birth of " + getName ());
			setBirthEvent (event);
		}

		if (date.isValid())
			return event->setDated(Dated(date));
		else
			return event->setDated(value.toString());
	}
	case e_col_marriageDate:
		if (isFemale() && spouseID)
		{
			QDate date = value.toDate();

			Event *event = getMarriageEvent(spouseID);
			if (!event)
			{
				event = new Event;
				Individual *spouse = s_IdToIndividualHash.value (spouseID);
				event->setName ("Marriage of " + getFullName () + ", and "
						+ spouse->getName ());
				setMarriageEvent (spouseID, event);
			}

			if (date.isValid())
				return event->setDated(Dated(date));
			else
				return event->setDated(value.toString());
		}
        return false;
	case e_col_deathDate:
	{
		QDate date = value.toDate();

		Event *event = getDeathEvent ();
		if (!event)
		{
			event = new Event;
			event->setName ("Death of " + getName ());
			setDeathEvent (event);
		}

		if (date.isValid())
			return event->setDated(Dated(date));
		else
			return event->setDated(value.toString());
	}
	case e_col_facebookID: return setFacebookID (value.toString ());
	case e_col_dummyOrBase:
	case e_col_individualIsDummy:
		return setDummy(value.toBool());
	default:
#ifdef DEBUGMODE
		qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
		break;
	}

    return Familiar::setData(col, value, spouseID);
}

QString Individual::getListOfAnchors (const QList<Individual*> &individuals,
				      const bool useFullName,
				      const bool createAnchor)
{
	QList <IdableWithChildren*> iids;
	foreach (Individual *individual, individuals)
		iids << dynamic_cast <IdableWithChildren*> (individual);

	return IdableWithChildren::getListOfAnchors (iids, useFullName, createAnchor);
}

QString Individual::getDecoration(const int col, const unsigned long spouseID) const
{
	switch (col)
	{
	case e_col_name:
		if (isFemale())
		{
			if (!spouseID)
			{
				if (hasSpouse())
					return ":images/daughter_married.png";
				else
					return ":images/daughter_unmarried.png";
			}
			else
				return ":images/wife.png";
		}
		else
		{
			if (hasSpouse())
			{
				if (hasOffspring())
					return ":images/son_with_children.png";
				else
					return ":images/son_married.png";
			}
			else
				return ":images/son_unmarried.png";
		}
	case e_col_sex:
		if (isFemale())
			return ":images/female.png";
		else
			return ":images/male.png";
	case e_col_dead:
		if (isDead())
			return ":images/dead.png";
		else
			return ":images/alive.png";
	/*case e_col_facebookID:
		if (hasFacebookID ())
			return ":images/facebook.png";*/
	case e_col_icon:
		qDebug () << getImagePath ();
		return getImagePath ();
	default:
#ifdef DEBUGMODE
		qDebug() << "Code reaches here:" << __FILE__ << __LINE__;
#endif
		break;
	}

	return "";
}

QString Individual::getAnchor (const IDTYPE spouseIdForCurrentEntryOfFemale, bool useFullName,
			       const bool createAnchor)
{
	if (createAnchor)
	{
		QString toolTip = getToolTip (false, spouseIdForCurrentEntryOfFemale);
		toolTip.replace ("<", "&lt;");
		toolTip.replace (">", "&gt;");
		return QString("<a href=\"iid:%1%4\" title=\"%3\">%2</a>")
				.arg(_id).arg(useFullName ? getFullName() : getUnderlinedName ())
				.arg(toolTip)
				.arg(spouseIdForCurrentEntryOfFemale ? QString ("#%1").arg(spouseIdForCurrentEntryOfFemale) : "");
	}

	if (useFullName)
		return "<u>" + getFullName() + "</u>";

	return getUnderlinedName();
}

QString Individual::getListOfAnchors(const int type, const bool useFullName,
				     const bool createAnchor)
{
	switch (AnchorType(type))
	{
	case bornTo:
		if (getFather () && getMother () && _family)
		{
			Individual *father = getFather ();
			Individual *mother = getMother ();
			return QString (" to %1%2 in the %3 family")
					.arg(father->getAnchor(0, useFullName, createAnchor))
					.arg(mother->isDummy() ? ""
						: QString (" and %1")
						.arg(mother->getAnchor(0, useFullName, createAnchor)))
					.arg(_family->getAnchor(0, useFullName, createAnchor));
		}
		break;
	case spouses:
		if (hasSpouse())
		{
			QList <IdableWithChildren *> spouseList;
			int nonDummySpouseCount = 0;
			for (int i = 0; i < spouseCount(); i++)
			{
				Individual *spouse = getSpouse (i);
				if (spouse->isDummy ())
					continue;
				nonDummySpouseCount ++;
				spouseList.append(spouse);
			}
			QString ret;
			ret = getAnchor(0, useFullName, createAnchor) + " ";
			ret += deathBias(hashad, false) + " ";
			ret += spouseCountAsString () + ".";
			if (!nonDummySpouseCount)
				return ret;

			ret += " " + genderBias(hisher, true) + " ";
			ret += pluralBias(spousespouses, spouseCount(), false) + " ";
			if (_dead)
				ret += pluralBias(waswere, spouseCount(), false);
			else
				ret += pluralBias(isare, spouseCount(), false);
			ret += " ";
			ret += IdableWithChildren::getListOfAnchors(spouseList, useFullName,
								    createAnchor);
			ret += ".";
			if (nonDummySpouseCount < spouseCount ())
				ret += " (A few spouses are not known.)";
			return ret;
		}
		break;
	case spousesWithOffspring:
		{
			QString ret = getListOfAnchors(spouses, useFullName, createAnchor);

			for (int i = 0; i < spouseCount(); i++)
			{
				Individual *spouse = getSpouse(i);
				if (offspringCount(spouse->getId()))
				{
					QList <IdableWithChildren *> offspringList;
					for (int i = 0; i < offspringCount(spouse->getId()); i++)
						offspringList.append(getOffspring(i, spouse->getId()));

					ret += " ";
					ret += genderBias(heshe, true) + " ";
					ret += deathBias(iswas, false) + " the ";
					ret += genderBias(fathermother, false) + " of ";
					ret += IdableWithChildren::getListOfAnchors(offspringList, useFullName, createAnchor);
					if (!spouse->isDummy ())
					{
						ret += " with ";
						ret += genderBias(hisher, false) + " spouse ";
						ret += spouse->getAnchor(_id, useFullName, createAnchor);
					}
					ret += ".";
				}
				else
				{
					ret += " ";
					ret += genderBias(heshe, true) + " ";
					ret += deathBias(hashad, false) + " no children with ";
					ret += genderBias(hisher, false) + " spouse ";
					ret += spouse->getAnchor(_id, useFullName, createAnchor) + ".";
				}
			}

			return ret;
		}
		break;
	case siblings:
		if (hasSibling())
		{
			QString ret = genderBias(heshe, true) + " ";
			ret += deathBias(hashad, false) + " ";
			ret += siblingCountAsString () + ".";
			if (hasElderSibling())
			{
				QList <IdableWithChildren *> siblingList;
				for (int i = 0; i <= lastElderSibling(); i++)
					siblingList.append(getSibling(i));
				ret += " ";
				ret += genderBias(heshe, true) + " ";
				ret += deathBias(iswas, false) + " the younger ";
				ret += genderBias(brothersister, false) + " of ";
				ret += IdableWithChildren::getListOfAnchors (siblingList, useFullName, createAnchor);
				ret += ".";
			}
			if (hasYoungerSibling())
			{
				QList <IdableWithChildren *> siblingList;
				for (int i = firstYoungerSibling(); i < siblingCount() + 1; i++)
					siblingList.append(getSibling(i));
				ret += " ";
				ret += genderBias(heshe, true) + " ";
				ret += deathBias(iswas, false) + " the elder ";
				ret += genderBias(brothersister, false) + " of ";
				ret += IdableWithChildren::getListOfAnchors(siblingList, useFullName, createAnchor);
				ret += ".";
			}
			return ret;
		}
		break;
	}
	return "";
}

QString Individual::getToolTip (const bool createAnchor, const IDTYPE spouseIdForCurrentEntryOfFemale)
{
	QString imageRight = "";

	QString bornToString = "";
	bornToString = getListOfAnchors (bornTo, false, createAnchor);

	QString namex = getFullName();
	namex += " <img src='";
	namex += getDecoration(e_col_name, spouseIdForCurrentEntryOfFemale);
	namex += "' title='";
	namex += data (e_col_sex, Qt::ToolTipRole, spouseIdForCurrentEntryOfFemale).toString();
	namex += "'> <img src='";
	namex += getDecoration(e_col_dead, spouseIdForCurrentEntryOfFemale);
	namex += "' title='";
	namex += data (e_col_dead, Qt::ToolTipRole, spouseIdForCurrentEntryOfFemale).toString();
	namex += "'> <img src='";
	namex += getDecoration(e_col_sex, spouseIdForCurrentEntryOfFemale);
	namex += "'>";

	namex += " <a href='" + getFacebookURL () + "'><img title='" + getFacebookText ()
			+ "' src=':images/facebook.png' height='20' /></a>";

	if (createAnchor)
	namex += QString(" <a href='edit:%1' title='Edit this entry'>"
			 "<img src=':images/editentry.png' height='20'></a>").arg(_id);
	QString aliases = "";
	if (_aliases.count())
	{
		aliases += "<abbr title='Also Known As'>aka</abbr> <i>" + _aliases.at(0);
		if (_aliases.count() > 1)
		{
			for (int i = 1; i < _aliases.count() - 1; i++)
				aliases += ", " + _aliases.at(i);
			aliases += ", and " + _aliases.at(_aliases.count() - 1);
		}
		aliases += "</i>";
	}
	QString name = "<h2>" + namex + "</h2>";
	if (_village.length())
		name += " (village <i>" + _village + "</i>";
	name += aliases;

	QString birthDeathInfoString;

	if (_birthEvent || _deathEvent)
	{
		QString place = "";
		if (_birthEvent)
			place = _birthEvent->getPlace().length() ?
				QString(" at <b>%1</b>").arg(_birthEvent->getPlace()) : "";

		int age;

		QDate today = QDate::currentDate();
		if (_dead && _deathEvent)
		{
			int deathAge = Dated(today) - _deathEvent->getDated();
			QString ago = deathAge > 1 ? QString(", <b>%1</b> years ago").arg(deathAge)
				      : (deathAge > 0 ? QString(", <b>one</b> year ago") : "");
			if (_birthEvent)
			{
				age = _deathEvent->getDated() - _birthEvent->getDated();
				if (age >= 0)
				{
					QString young = age < 55 ? " young" : "";
					birthDeathInfoString = QString ("%1 was born%2 on <b>%3</b>%4 and died at the%5 age of %6 on <b>%7</b>%8.")
							       .arg(name)
							       .arg(bornToString)
							       .arg(_birthEvent->toDisplayDate())
							       .arg(place)
							       .arg(young)
							       .arg(age > 1 ? QString ("<b>%1</b> years").arg(age) : "<b>one</b> year")
							       .arg(_deathEvent->toDisplayDate())
							       .arg(ago);
				}
				else
				{
					birthDeathInfoString = QString ("%1 was born%2 on <b>%3</b>%4 and died on <b>%5</b>%6.")
							       .arg(name)
							       .arg(bornToString)
							       .arg(_birthEvent->toDisplayDate())
							       .arg(place)
							       .arg(_deathEvent->toDisplayDate())
							       .arg(ago);
				}
			}
			else
			{
				birthDeathInfoString = QString ("%1 was born%2 and died on <b>%5</b>%6.")
						.arg(name)
						.arg(bornToString)
						.arg(_deathEvent->toDisplayDate())
						.arg(ago);
			}
		}
		else if (_birthEvent)
		{
			age = Dated (today) - _birthEvent->getDated();

			QString ago = _dead ?
				      (age > 1 ? QString(", <b>%1</b> years ago. Sadly though he is no longer with us").arg(age)
				       : (age > 0 ? ", <b>one</b> year ago. Sadly though he is no longer with us" : ""))
				      :
				      (age > 1 ? QString(". %1 is currently <b>%2</b> years old").arg(genderBias(heshe, true)).arg(age)
				       : (age > 0 ? QString(". %1 is currently <b>one</b> year old.").arg(genderBias(heshe, true)) : ""));

			birthDeathInfoString = QString("%1 was born%2 on <b>%3</b>%4%5.")
					       .arg(name)
					       .arg(bornToString)
					       .arg(_birthEvent->toDisplayDate())
					       .arg(place)
					       .arg(ago);
		}
	}
	else if (bornToString.length())
		birthDeathInfoString = name + " was born " + bornToString.replace(",", ".") + ".";
	else
		birthDeathInfoString = name;

	if (hasSibling())
		birthDeathInfoString += " " + getListOfAnchors(siblings, false, createAnchor);

	QString marriageString = "";
	if (hasSpouse())
		marriageString = getListOfAnchors(spousesWithOffspring, false, createAnchor);

	imageRight += "<p>" + birthDeathInfoString + "</p>";
	imageRight += "<p>" + marriageString + "</p>";

	QString ret = "";
	ret += QString ("<a href='image:%1'><img src='%2' width='%3' title='%4' align='left' /></a>"
			"<p>%5</p>")
			.arg(_id).arg(getImagePath ())
			.arg(SettingsManager::s_MaxEditDialogImageWidth)
			.arg((_imagePath.isEmpty() ? "Place for image of " : "Image of ") + getName ())
			.arg(imageRight);

	if (!_statsUpToDate)
		updateStatistics ();

	QString selfLink = getAnchor(spouseIdForCurrentEntryOfFemale, false, createAnchor);

	if (!isFemale ())
		ret += "<p>" + selfLink +  getStatisticsHTML () + "</p>";

	return ret;
}

QString Individual::getItemViewPage (const IDTYPE spouseIdForCurrentEntryOfFemale)
{
	if (isDummy())
		return "This is a dummy individual. We don't know much about this individual."
				" Remove the associated dummy tag to view/edit the information.";

	QString ret = "<table width=\"100%\" border=\"0\">";
    ret += "<tr><td>" + Familiar::getItemViewPage(spouseIdForCurrentEntryOfFemale) + "</td></tr>";

	if (SettingsManager::s_DisplayIndianRelations)
	{
		QString relativeDisplayString = getRelativesAsDisplayString ();
		if (relativeDisplayString.size ())
			ret += "<tr><td>" + relativeDisplayString + "</td></tr>";
	}

	QList<Event> eventList;
	foreach (Event *event, Event::s_EventList)
		if (event->hasEventMember(_id))
			eventList.append(*event);

	if (eventList.size())
	{
        std::sort(eventList.begin(), eventList.end());

		ret += "<tr><td><h3>Event Information</h3>";
		ret += "<ol>";

		foreach (Event event, eventList)
			ret += "<li>" + event.getEventInformation() + "</li>";

		ret += "</ol></td></tr>";
	}

	if (_notes.length())
	{
		QString convertedNotes;
		Utilities::convertFTMLToDisplayFormat(_notes, convertedNotes, false, true, -1, this);
		ret += "<tr><td><hr><h3>Notes</h3>" + convertedNotes + "</td></tr>";
	}

	ret += "</table>";

	return ret;
}
