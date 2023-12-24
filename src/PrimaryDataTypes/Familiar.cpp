#include "Familiar.h"
#include <QColor>
#include <QFont>

Familiar::Familiar (const IDTYPE id,
		    const QDateTime &creationDateTime, const QDateTime &modificationDateTime,
		    const unsigned long modificationCount)
			    :IdableWithChildren (id, creationDateTime, modificationDateTime, modificationCount)
{
	_refCount = 0;

	_modelSetupInProgress = false;
	_statsUpToDate = false;
	_sonCount = 0;
	_daughterCount = 0;
	_marriedDaughterCount = 0;
	_spouseCount = 0;
	_deadCount = 0;
	_generations = 0;
	_totalCount = 0;
}

Familiar::Familiar (const QDomElement &domElement) : IdableWithChildren (domElement)
{
	_refCount = 0;

	QDomNodeList aliasNodeList = domElement.elementsByTagName("Alias");
	for (int i = 0; i < aliasNodeList.count(); i++)
	{
		QDomNode aliasNode = aliasNodeList.at(i);
		QString alias = aliasNode.toElement().text().trimmed();
		unescapeStringFromXML(alias);
		_aliases.append(alias);
	}
}

Familiar::Familiar (const Familiar &Familiar) : IdableWithChildren(Familiar)
{
	_refCount = 0;
	_treeItem = 0;
    _aliases = Familiar._aliases;
    _statsUpToDate = Familiar._statsUpToDate;
    _sonCount = Familiar._sonCount;
    _daughterCount = Familiar._daughterCount;
    _marriedDaughterCount = Familiar._marriedDaughterCount;
    _spouseCount = Familiar._spouseCount;
    _deadCount = Familiar._deadCount;
    _generations = Familiar._generations;
    _totalCount = Familiar._totalCount;
}

Familiar::~Familiar ()
{
}

void Familiar::setupItemLinks(const QDomElement &domElement)
{
	Q_UNUSED (domElement)

	IdableWithChildren::setupItemLinks (domElement);
	unsetStatsUpToDate();
}

void Familiar::setupModel(TreeModel *model, int row, QModelIndex &parent, QProgressDialog &progress)
{
	Q_UNUSED(model)
	Q_UNUSED(row)
	Q_UNUSED(parent)

	progress.setValue(progress.value() + 1);
}

/*bool Familiar::isValid () const
{
	return IdableWithChildren::isValid();
}*/

SETFUNCTIONWITHCONSTCAST (Familiar, TreeItem, TreeItem *, treeItem)
SETFUNCTION (Familiar, Aliases, QStringList, aliases)

int Familiar::columnCount()
{
	return e_col_maximum;
}

QVariant Familiar::headerData(int section, Qt::Orientation orientation, int role)
{
	if (orientation == Qt::Horizontal)
	{
		t_columns column = static_cast<t_columns>(section);
		if (role == Qt::DisplayRole)
			switch (column)
			{
			case e_col_name: return "Name";
			case e_col_alias: return "Pet Name";
			case e_col_sex: return "Sex";
			case e_col_male: return "Is Male?";
			case e_col_female: return "Is Female?";
			case e_col_dead: return "Dead";
			case e_col_birthDate: return "Birth Date";
			case e_col_deathDate: return "Death Date";
			case e_col_marriageDate: return "Marriage Date";
			case e_col_link: return "Links";
			case e_col_facebookID: return "Facebook ID";
			case e_col_facebookURL: return "Facebook URL";
			case e_col_myGeneration: return "Generation";
			case e_col_dummyOrBase: return "Dummy/Base";
			case e_col_isIndividual: return "Is Individual?";
			case e_col_individualIsDummy: return "Individual is Dummy?";
			case e_col_isFamily: return "Is Family?";
			case e_col_familyIsBase: return "Family is Base?";
			case e_col_hasBirthEvent: return "Has Birth Event?";
			case e_col_hasDeathEvent: return "Has Death Event?";
			case e_col_hasMarriageEvent: return "Has Marriage Event?";
			case e_col_sonCount: return "S.C.";
			case e_col_daughterCount: return "D.C.";
			case e_col_marriedDaughterCount: return "M.D.C.";
			case e_col_spouseCount: return "Sp.C.";
			case e_col_aliveCount: return "A.C.";
			case e_col_deadCount: return "D.C.";
			case e_col_totalCount: return "T.C.";
			case e_col_generations: return "Generations";
			case e_col_icon: return "Icon";
			default:
				break;
			}
	}

	return QVariant();
}

Qt::ItemFlags Familiar::flags (const int col, const bool isSpouseItem) const
{
	Q_UNUSED(isSpouseItem)

	Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;

	switch (col)
	{
	case e_col_name:
		itemFlags |= Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
		break;
	case e_col_alias:
		itemFlags |= Qt::ItemIsEditable;
		break;
	case e_col_sonCount:
	case e_col_daughterCount:
	case e_col_marriedDaughterCount:
	case e_col_spouseCount:
	case e_col_aliveCount:
	case e_col_deadCount:
	case e_col_totalCount:
	case e_col_generations:
	case e_col_isIndividual:
	case e_col_individualIsDummy:
	case e_col_isFamily:
	case e_col_familyIsBase:
	case e_col_hasBirthEvent:
	case e_col_hasDeathEvent:
	case e_col_hasMarriageEvent:
		break;
	default:
        itemFlags = Qt::NoItemFlags;
		break;
	}

	return itemFlags;
}

QVariant Familiar::data (const int col, const int role, const IDTYPE spouseID)
{
	Q_UNUSED(spouseID)

	if (role == Qt::ForegroundRole)
	{
		switch (col)
		{
		case e_col_alias: return QColor ("gray");
		default:
			break;
		}
	}
	else if (role == Qt::FontRole)
	{
		QFont font;
		switch (col)
		{
		case e_col_alias:
			font.setItalic(true);
			break;
		default:
			break;
		}
		return font;
	}
	else if (role == Qt::WhatsThisRole)
	{
		switch (col)
		{
		case e_col_alias:
			return QVariant(QString("<b>Pet Name</b> of the individual/family."
						"<br><br>This field can be edited inline."));
		default:
			break;
		}
	}
	else if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch (col)
		{
		case e_col_name: return _name;
		case e_col_alias: return getAliasesAsCommaSeparatedList (true);
		case e_col_sonCount: return _sonCount;
		case e_col_daughterCount: return _daughterCount;
		case e_col_marriedDaughterCount: return _marriedDaughterCount;
		case e_col_spouseCount: return _spouseCount;
		case e_col_aliveCount: return _totalCount - _deadCount;
		case e_col_deadCount: return _deadCount;
		case e_col_totalCount: return _totalCount;
		case e_col_generations: return _generations;
		case e_col_isIndividual:
		case e_col_individualIsDummy:
		case e_col_isFamily:
		case e_col_familyIsBase:
		case e_col_hasBirthEvent:
		case e_col_hasDeathEvent:
		case e_col_hasMarriageEvent:
			return false;
		default:
			break;
		}
	}
	else if (role == Qt::UserRole)
	{
		if (col == e_col_name)
		{
			return QString("%1").arg (getId ());
		}
	}		/// Qt::UserRole

	return QVariant();
}

bool Familiar::setData (const int col, const QVariant &value, const unsigned long spouseID)
{
	Q_UNUSED(spouseID)

	switch (col)
	{
	case e_col_name: return setName(value.toString());
	case e_col_alias: return setAliasesFromCommaSeparatedList(value.toString());
	default:
		break;
	}

	return false;
}

QTextStream& operator << (QTextStream &stream, Familiar &Familiar)
{
    stream << dynamic_cast <IdableWithChildren &> (Familiar);
    foreach (QString alias, Familiar._aliases)
	{
		Idable::escapeStringForXML (alias);
        stream << "\t\t<Alias>" << alias << "</Alias>" << Qt::endl;
	}

	return stream;
}

QDebug operator << (QDebug debug, Familiar &Familiar)
{
    foreach (QString alias, Familiar._aliases)
        debug << "Alias:" << alias << Qt::endl;

    debug << dynamic_cast <IdableWithChildren &> (Familiar);

    debug << "Are Stats up to date:" << Familiar._statsUpToDate << Qt::endl;
    debug << "Son Count:" << Familiar._sonCount << Qt::endl;
    debug << "Daughter Count:" << Familiar._daughterCount << Qt::endl;
    debug << "Married Daughter Count:" << Familiar._marriedDaughterCount << Qt::endl;
    debug << "Spouse Count:" << Familiar._spouseCount << Qt::endl;
    debug << "Dead Count:" << Familiar._deadCount << Qt::endl;
    debug << "Generations:" << Familiar._generations << Qt::endl;
    debug << "Total Count:" << Familiar._totalCount << Qt::endl;

	return debug;
}

bool Familiar::operator == (const Familiar & oFamiliar) const
{
    return getName() == oFamiliar.getName ();
}

bool Familiar::operator != (const Familiar & oFamiliar) const
{
    return getName() != oFamiliar.getName ();
}

QString Familiar::pluralBias(const EnglishWord ew, const int count, const bool capitalise) const
{
	QString temp;
	switch (ew)
	{
	case isare:
		temp = (count == 1 ? "is" : "are");
		break;
	case waswere:
		temp = (count == 1 ? "was" : "were");
		break;
	case spousespouses:
		temp = (count == 1 ? "spouse" : "spouses");
		break;
	case membermembers:
		temp = (count == 1 ? "member" : "members");
		break;
	case generationgenerations:
		temp = (count == 1 ? "generation" : "generations");
		break;
	default:
		break;
	}

	if (capitalise > 0)
		Idable::toTitleCase(temp);
	return temp;
}

QString Familiar::getAliasesAsCommaSeparatedList (bool useAndForLastString) const
{
	QString ret = "";
	if (_aliases.size())
	{
		ret = _aliases.at(0);
		if (_aliases.size() > 1)
		{
			for (int i = 1; i < _aliases.size() - 1; i++)
				ret += ", " + _aliases.at(i);
			ret += ", ";
			if (useAndForLastString)
				ret += "and";
			ret += _aliases.at(_aliases.size() - 1);
		}
	}
	return ret;
}

bool Familiar::setAliasesFromCommaSeparatedList(QString aliases)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    const auto skipEmptyParts = Qt::SkipEmptyParts;
#else
    const auto skipEmptyParts = QString::SkipEmptyParts;
#endif

    QStringList aliasesList = aliases.split (",", skipEmptyParts);
	aliasesList.replaceInStrings (QRegExp ("^\\s+"), "");
	aliasesList.replaceInStrings (QRegExp ("\\s+$"), "");
	if (aliasesList == _aliases)
		return false;

	_aliases = aliasesList;

	updateModificationTime();
	return true;
}

void Familiar::getStatistics (unsigned long &sonCount, unsigned long &daughterCount,
			      unsigned long &marriedDaughterCount, unsigned long &spouseCount,
			      unsigned long &deadCount, unsigned long &generations)
{
	if (!_statsUpToDate)
		updateStatistics ();

	sonCount = _sonCount;
	daughterCount = _daughterCount;
	marriedDaughterCount = _marriedDaughterCount;
	spouseCount = _spouseCount;
	deadCount = _deadCount;
	generations = _generations;
}

QString Familiar::getStatisticsHTML () const
{
	QString ret = "";
	if (_totalCount)
	{
		ret += " has in total <b>" +
		       toStringRepresentationOfNumber (_totalCount, "member", "members") + "</b> in <b>" +
		       toStringRepresentationOfNumber (_generations, "generation", "generations") + "</b>.";

		QString heirString = "";
		if (_sonCount)
			heirString += toStringRepresentationOfNumber (_sonCount, "male heir", "male heirs") +
				      QString (" (%1%)").arg (100 * _sonCount / _totalCount);
		if (_daughterCount)
		{
			heirString += ", " +
				      toStringRepresentationOfNumber (_daughterCount - _marriedDaughterCount,
								      "female heir", "female heirs") +
				      QString(" (%1%)").arg(100 * (_daughterCount - _marriedDaughterCount) / _totalCount);
			if (_spouseCount)
				heirString += ", and " +
					      toStringRepresentationOfNumber (_spouseCount, "spouse", "spouses") +
					      QString (" (%1%) ").arg (100 * _spouseCount / _totalCount) +
					      " married into family.";
			if (_marriedDaughterCount)
				heirString +=  " And just for the record, " +
					       toStringRepresentationOfNumber (_marriedDaughterCount, "daughter", "daughters") +
					       QString (" (%1%)").arg (100 * _marriedDaughterCount / _daughterCount) +
					       ", have been married (out of the family),"
					       " and hence, have not been included in the"
					       " total count.";
		}

		if (heirString.length ())
			ret += " This tree has " + heirString;

		if (_deadCount)
		{
			ret += " A total of " +
			       toStringRepresentationOfNumber (_deadCount, "member", "members") +
			       QString (" (%1%) ").arg (100*_deadCount/_totalCount) +
			       pluralBias (isare, _deadCount, false) +
			       " already dead.";
		}
	}
	return ret;
}

/*void Familiar::operator + (const unsigned long &chid)
{
	appendChild (chid);
}

void Familiar::operator - (const unsigned long &chid)
{
	removeChild (chid);
}*/

QString Familiar::getItemViewPage (const IDTYPE spouseIdForCurrentEntryOfFemale)
{
	return getToolTip(true, spouseIdForCurrentEntryOfFemale);
}

bool Familiar::fullTextSearch (const QRegExp &searchRegExp, QStringList &resultsList) const
{
	bool found = IdableWithChildren::fullTextSearch (searchRegExp, resultsList);

	for (int i = 0; i < _aliases.count (); i++)
	{
		if (_aliases.at (i).contains (searchRegExp))
			resultsList << QString ("Alias: %1").arg (_aliases.at (i));
	}

	return found;
}
