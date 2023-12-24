#include "Individual.h"
#include "Union.h"
#include <QColor>
#include <QPixmap>
#include <QFont>
#include "Utilities.h"
#include "SettingsManager.h"

#define UPDATEINHERITABLEVARIABLE(variable,capitalized) \
	_ ## variable = domElement.attribute(# variable, "").trimmed(); \
	unescapeStringFromXML(_ ## variable); \
	if (_ ## variable.length()) \
		_is ## capitalized ## Inherited = false; \
	else \
		_is ## capitalized ## Inherited = true;


DEFINEAUTOITEMMANAGEMENT(Individual)

uint Individual::s_DummyIndividualCount = 0;

Individual::Individual (const IDTYPE id,
			const QDateTime &creationDateTime, const QDateTime &modificationDateTime,
			const unsigned long modificationCount)
                : Familiar (id, creationDateTime, modificationDateTime, modificationCount)
{
	init();
}

Individual::Individual (const QDateTime &creationDateTime, const QDateTime &modificationDateTime,
			const unsigned long modificationCount)
                : Familiar (++s_MaxIndividualId, creationDateTime, modificationDateTime, modificationCount)
{
	init ();
}

Individual::Individual (const QDomElement &domElement) : Familiar (domElement)
{
	_dummy = domElement.attribute("dummy", "0").trimmed().toULong();
	if (_dummy)
		s_DummyIndividualCount ++;
	_dead = domElement.attribute("dead", "0").trimmed().toULong();

	UPDATEINHERITABLEVARIABLE(village,Village)
	UPDATEINHERITABLEVARIABLE(address,Address)

	_imagePath = domElement.attribute("imagePath", "").trimmed();
	unescapeStringFromXML(_imagePath);

	_facebookID = domElement.attribute ("facebookID", "").trimmed ();
	unescapeStringFromXML (_facebookID);

	if (domElement.elementsByTagName("Notes").count())
	{
		_notes = domElement.elementsByTagName("Notes").at(0).toElement().text().trimmed();
		unescapeStringFromXML(_notes);
	}
	else
		_notes = "";

	initLinks();

	_thumbnail = NULL;
	_pixmap = NULL;
}

Individual::Individual (const Individual &individual) : Familiar (individual)
{
	_dummy = individual._dummy;
	_dead = individual._dead;
	_adoptedTo = individual._adoptedTo;
	_adoptedFrom = individual._adoptedFrom;
	_parentUnion = individual._parentUnion;
	_birthEvent = individual._birthEvent;
	_deathEvent = individual._deathEvent;
	_family = individual._family;
	_isFamilyInherited = individual._isFamilyInherited;
	_village = individual._village;
	_isVillageInherited = individual._isVillageInherited;
	_address = individual._address;
	_isAddressInherited = individual._isAddressInherited;
	_imagePath = individual._imagePath;
	_facebookID = individual._facebookID;
	_notes = individual._notes;

    _thumbnail = individual._thumbnail;
    _pixmap = individual._pixmap;
}

Individual::~Individual ()
{
	REMOVEITEMFROMITEMMANAGEMENT(Individual)

	if (!_copyConstructed)
	{
		if (_dummy)
			s_DummyIndividualCount --;

		if (_pixmap)
			delete _pixmap;
		if (_thumbnail)
			delete _thumbnail;
	}
}

void Individual::setupItemLinks(const QDomElement &domElement)
{
    Familiar::setupItemLinks(domElement);

	LOADITEMFROMXMLWITHOUTCREATION(Individual,adoptedTo)
	LOADITEMFROMXMLWITHOUTCREATION(Individual,adoptedFrom)
	LOADITEMFROMXMLWITHOUTCREATION(Event,birthEvent)
	LOADITEMFROMXMLWITHOUTCREATION(Event,deathEvent)
}

/*bool Individual::isValid () const
{
    return Familiar::isValid();
}*/

void Individual::updateAdoptedIndividual (Individual *adoptedIndividual)
{
	IDTYPE id = adoptedIndividual->getId ();
	*adoptedIndividual = *this;
	_adoptedTo = adoptedIndividual;
	_pixmap = NULL;
	_thumbnail = NULL;
	adoptedIndividual->_adoptedFrom = this;
	adoptedIndividual->_parentUnion = NULL;
	adoptedIndividual->_family = NULL;
	adoptedIndividual->setId (id);
}

DEFINITIONOFGETITEMWITHID(Individual,Union)

void Individual::init ()
{
	_dummy = false;
	_dead = false;

	_isFamilyInherited = true;
	_isAddressInherited = true;
	_isVillageInherited = true;

	_thumbnail = NULL;
	_pixmap = NULL;

	initLinks();
}

void Individual::initLinks ()
{
	_adoptedTo = 0;
	_adoptedFrom = 0;

	_parentUnion = 0;
	_birthEvent = 0;
	_deathEvent = 0;
	_family = 0;

	INSERTITEMINTOITEMMANAGEMENT(Individual)
}

bool Individual::isDead() const
{
	if (_adoptedTo)
		return _adoptedTo-> isDead();

	return _dead;
}

bool Individual::isFamilyInherited() const
{
	if (_adoptedTo)
		return _adoptedTo->isFamilyInherited();

	return _isFamilyInherited;
}

bool Individual::isVillageInherited() const
{
	if (_adoptedTo)
		return _adoptedTo->isVillageInherited();

	return _isVillageInherited;
}

bool Individual::isAddressInherited() const
{
	if (_adoptedTo)
		return _adoptedTo->isAddressInherited();

	return _isAddressInherited;
}

Individual* Individual::getFather () const
{
	if (!_parentUnion)
		return NULL;

	return getParentUnion ()->getHusband ();
}

Individual* Individual::getMother () const
{
	if (!_parentUnion)
		return NULL;

	return getParentUnion ()->getWife ();
}

QString Individual::getImagePath () const
{
	if (_adoptedTo)
		return _adoptedTo->getImagePath ();

	if (_imagePath.size())
		return SettingsManager::s_DocumentDirectoryPath + "/" + _imagePath;

	if (isFemale())
		return ":images/female_icon.png";
	else
		return ":images/male_icon.png";
}

SETFUNCTIONWITHCONSTCAST (Individual,	AdoptedTo,	Individual *,	adoptedTo)
SETFUNCTIONWITHCONSTCAST (Individual,	AdoptedFrom,	Individual *,	adoptedFrom)

bool Individual::setDummy (const bool dummy)
{
	/* No setting dummy for males and adopted individuals.*/
	if (!isFemale () || _adoptedTo || _adoptedFrom)
		return false;

	if (_dummy == dummy)
		return false;

	_dummy = dummy;
	if (_dummy)
	{
		_name = QString ("Dummy-%1").arg(_id);
		s_DummyIndividualCount ++;
	}
	else
	{
		_name = FIXME;
		s_DummyIndividualCount --;
	}
	updateModificationTime ();
	return true;
}

GETSETMAXIDCPP (Individual)

SETFUNCTIONI (Individual,	Dead,		bool,		dead)

SETFUNCTIONWITHCONSTCAST (Individual,	ParentUnion,	Union *,	parentUnion)
SETFUNCTIONWITHCONSTCASTI (Individual,	BirthEvent,	Event *,	birthEvent)
SETFUNCTIONWITHCONSTCASTI (Individual,	DeathEvent,	Event *,	deathEvent)
SETFUNCTIONWITHCONSTCAST (Individual,	Family,		Family *,	family)

bool Individual::setVillageInherited(const bool villageInherited)
{
	if (_adoptedTo)
		return _adoptedTo->setVillageInherited (villageInherited);

	if (_isVillageInherited == villageInherited)
		return false;

	_isVillageInherited = villageInherited;
	if (villageInherited)
	{
		Individual *ancestor = getFather ();
		while (ancestor)
		{
			_village = ancestor->getVillage();
			if (ancestor->isVillageInherited())
				ancestor = ancestor->getFather();
			else
				break;
		}
	}
	updateModificationTime();
	return true;
}

SETFUNCTIONIQSTRING (Individual,	Village,	village)

bool Individual::setAddressInherited(const bool addressInherited)
{
	if (_adoptedTo)
		return _adoptedTo->setAddressInherited (addressInherited);

	if (_isAddressInherited == addressInherited)
		return false;

	_isAddressInherited = addressInherited;
	if (addressInherited)
	{
		Individual *ancestor = getFather ();
		while (ancestor)
		{
			_village = ancestor->getAddress();
			if (ancestor->isAddressInherited())
				ancestor = ancestor->getFather();
			else
				break;
		}
	}
	updateModificationTime();
	return true;
}

SETFUNCTIONIQSTRING (Individual,	Address,	address)

bool Individual::setImagePath (const QString &imagePath)
{
	if (_adoptedTo)
		return _adoptedTo->setImagePath (imagePath);

	QString timagePath = imagePath.trimmed ();
	if (_imagePath == timagePath)
		return false;

	_imagePath = timagePath;
	if (_thumbnail)
		delete _thumbnail;
	if (_pixmap)
		delete _pixmap;
	_thumbnail = NULL;
	_pixmap = NULL;

	updateModificationTime ();
	return true;
}

SETFUNCTIONIQSTRING (Individual,	FacebookID,	facebookID)
SETFUNCTIONIQSTRING (Individual,	Notes,		notes)

QString Individual::getChildTypeName () const
{
	return "Marriage";
}

QString Individual::getPetName () const
{
	if (_aliases.count())
		return _aliases.at(0);

	return "";
}

QString Individual::getSurName () const
{
	if (!_family)
		return "";

	return _family->getName ();
}

QString Individual::getFullName () const
{
	return _name + " " + getSurName ();
}

bool Individual::hasImage () const
{
	if (_adoptedTo)
		return _adoptedTo->hasImage ();

	return _imagePath.size () > 0;
}

QPixmap* Individual::getImage ()
{
	if (!_pixmap)
		_pixmap = new QPixmap (getImagePath ());
	return _pixmap;
}

QIcon* Individual::getIcon ()
{
	if (!_thumbnail)
		_thumbnail = new QIcon (getImagePath ());
	return _thumbnail;
}

bool Individual::hasFacebookID () const
{
	return getFacebookID ().size () > 0;
}

QString Individual::getFacebookURL () const
{
	QString facebookURL = "http://facebook.com/";
	if (hasFacebookID ())
		facebookURL += getFacebookID ();
	else
		facebookURL += "search/results.php?q=" + getFullName ();

	return facebookURL;
}

QString Individual::getFacebookText () const
{
	QString facebookURLText;
	if (hasFacebookID ())
		facebookURLText = "View";
	else
		facebookURLText = "Search";
	facebookURLText += " Facebook Profile for " + getFullName ();

	return facebookURLText;
}

bool Individual::setPetName(const QString &petName)
{
	QString tpetName = petName.trimmed();
	if (!tpetName.length())
		return false;

	if (_aliases.count() == 0)
		_aliases.append(tpetName);
	else
	{
		if (!_aliases.contains(tpetName))
			_aliases.insert(0, tpetName);
		else
            _aliases.swapItemsAt(0, _aliases.indexOf(tpetName));
	}

	updateModificationTime();
	return true;
}

bool Individual::hasSpouse () const
{
	return childCount() > 0;
}

bool Individual::isSpouse (const IDTYPE spid) const
{
	return getSpouseWithID (spid) != NULL;
}

long Individual::spouseCount () const
{
	return childCount ();
}

QString Individual::spouseCountAsString () const
{
	return toStringRepresentationOfNumber (spouseCount (), "spouse", "spouses");
}

long Individual::spouseNumber (const IDTYPE spid) const
{
	for (int i = 0; i < spouseCount(); i++)
	{
		Individual *spouse = getSpouse(i);
		if (spouse->getId() == spid)
			return i;
	}

	return -1;
}

Individual* Individual::getSpouse (const long spidx) const
{
	if (spidx < 0 || spidx >= childCount())
		return NULL;

	Union *aUnion = dynamic_cast<Union *> (childAt(spidx));
	return aUnion->getSpouse(!isFemale());
}

Individual* Individual::getSpouseWithID (const IDTYPE spid) const
{
	return getSpouse(spouseNumber (spid));
}

Union* Individual::getUnionWithSpouseID (const IDTYPE spid) const
{
	for (int i = 0; i < childCount(); i++)
	{
		Union *aUnion = dynamic_cast<Union *> (childAt(i));
		if (spid == aUnion->getSpouse(!isFemale())->getId())
			return aUnion;
	}
	return NULL;
}

bool Individual::hasOffspring () const
{
	return offspringCount() > 0;
}

bool Individual::isOffspring (const IDTYPE chid, const IDTYPE spid) const
{
	return getOffspringWithId (chid, spid) != NULL;
}

long Individual::offspringCount (const IDTYPE spid) const
{
	if (spid == 0)
	{
		long count = 0;
		for (int i = 0; i < childCount(); i++)
		{
			Union *aUnion = dynamic_cast<Union *> (childAt(i));
			count += aUnion->childCount();
		}
		return count;
	}
	else
	{
		Union *aUnion = getUnionWithSpouseID (spid);
		if (aUnion)
			return aUnion->childCount();
		else
			return -1;
	}
}

QString Individual::offspringCountAsString (const unsigned long spid) const
{
	return toStringRepresentationOfNumber (offspringCount (spid), "child", "children");
}

long Individual::offspringNumber (const IDTYPE spid, const IDTYPE chid) const
{
	Union *aUnion = getUnionWithSpouseID (spid);
	for (int i = 0; i < aUnion->childCount(); i++)
	{
		Individual *child = dynamic_cast<Individual *> (aUnion->childAt(i));
		if (chid == child->getId())
			return i;
	}

	return -1;
}

Individual* Individual::getOffspring (const long chidx, const IDTYPE spid) const
{
	if (chidx < 0)
		return NULL;

	Union *aUnion = getUnionWithSpouseID (spid);
	if (chidx >= aUnion->childCount())
		return NULL;

	return dynamic_cast<Individual *> (aUnion->childAt(chidx));
}

Individual* Individual::getOffspringWithId (const IDTYPE chid, const IDTYPE spid) const
{
	if (spid)
		return getOffspring (offspringNumber(spid, chid), spid);
	else
	{
		for (int i = 0; i < childCount(); i++)
		{
			Union *aUnion = dynamic_cast<Union *> (childAt(i));
			for (int j = 0; j < aUnion->childCount(); j++)
			{
				Individual *child = dynamic_cast<Individual *> (aUnion->childAt(j));
				if (chid == child->getId())
					return child;
			}
		}
		return NULL;
	}
}

Event* Individual::getMarriageEvent (const IDTYPE spid) const
{
	Union *aUnion = getUnionWithSpouseID (spid);
	if (aUnion)
		return aUnion->getMarriageEvent ();

	return NULL;
}

Event* Individual::getDivorceEvent (const IDTYPE spid) const
{
	Union *aUnion = getUnionWithSpouseID (spid);
	if (aUnion)
		return aUnion->getDivorceEvent ();

	return NULL;
}

Event* Individual::getMarriageEventIndex (const long spidx) const
{
	Individual *spouse = getSpouse (spidx);
	if (!spouse)
		return NULL;

	return getMarriageEvent (spouse->getId ());
}

Event* Individual::getDivorceEventIndex (const long spidx) const
{
	Individual *spouse = getSpouse (spidx);
	if (!spouse)
		return NULL;

	return getDivorceEvent (spouse->getId ());
}

bool Individual::setMarriageEvent(const IDTYPE spid, Event *marriageEvent)
{
	Union *aUnion = getUnionWithSpouseID (spid);
	if (aUnion)
		return aUnion->setMarriageEvent(marriageEvent);

	return false;
}

bool Individual::setDivorceEvent(const IDTYPE spid, Event *divorceEvent)
{
	Union *aUnion = getUnionWithSpouseID (spid);
	if (aUnion)
		return aUnion->setDivorceEvent(divorceEvent);

	return false;
}

bool Individual::hasSibling () const
{
	return siblingCount() > 0;
}

bool Individual::hasElderSibling () const
{
	return _parentUnion && _parentUnion->childNumber (this) != 0;
}

bool Individual::hasYoungerSibling () const
{
	return _parentUnion && (_parentUnion->childNumber (this) < _parentUnion->childCount () - 1);
}

long Individual::siblingCount () const
{
	if (!_parentUnion)
		return 0;

	return _parentUnion->childCount () - 1;
}

QString Individual::siblingCountAsString () const
{
	return toStringRepresentationOfNumber (siblingCount (), "sibling", "siblings");
}

long Individual::lastElderSibling () const
{
	if (!_parentUnion)
		return -1;

	return _parentUnion->childNumber (this)- 1;
}

long Individual::firstYoungerSibling () const
{
	if (!_parentUnion)
		return -1;

	long index = _parentUnion->childNumber (this) + 1;
	if (index >= _parentUnion->childCount ())
		return -1;
	return index;
}

Individual* Individual::getSibling (const long sbidx) const
{
	if (!_parentUnion)
		return NULL;

	if (sbidx < 0 || sbidx >= _parentUnion->childCount () ||
	    sbidx == _parentUnion->childNumber (this))
		return NULL;

	return dynamic_cast<Individual *>(_parentUnion->childAt (sbidx));
}

bool Individual::isOlderThan (const Individual *left, const Individual *right)
{
	try
	{
		return left->isOlderThan (right);
	}
	catch (NoBirthEventException &e)
	{
		if (e.getID () == left->getId ())
			return false;
		if (e.getID () == right->getId ())
			return true;
		return left->getName () > right->getName ();
	}
}

bool Individual::isYoungerThan (const Individual *left, const Individual *right)
{
	try
	{
		return right->isOlderThan (left);
	}
	catch (NoBirthEventException &e)
	{
		if (e.getID () == left->getId ())
			return false;
		if (e.getID () == right->getId ())
			return true;
		return left->getName () < right->getName ();
	}
}

bool Individual::isOlderThan (const Individual *individual) const
{
	Event *e1 = getBirthEvent ();
	Event *e2 = individual->getBirthEvent ();
	if (!e1 && !e2) throw (NoBirthEventException (0));
	if (!e1) throw (NoBirthEventException (getId ()));
	if (!e2) throw (NoBirthEventException (individual->getId ()));

	return *e1 > *e2;
}

QString Individual::deathBias(const EnglishWord ew, const bool capitalise) const
{
	QString temp;
	switch (ew)
	{
	case hashad:
		temp = (_dead ? "had" : "has" );
		break;
	case iswas:
		temp = (_dead ? "was" : "is");
		break;
	default:
		break;
	}

	if (capitalise)
		Idable::toTitleCase(temp);

	return temp;
}

bool Individual::fullTextSearch (const QRegExp &searchRegExp, QStringList &resultsList) const
{
    bool found = Familiar::fullTextSearch (searchRegExp, resultsList);

	if (_dummy && searchRegExp.pattern ().toLower ()== "dummy")
	{
		found = true;
		resultsList << "Dummy";
	}

	if (_dead)
	{
		if (searchRegExp.pattern ().toLower () == "dead")
		{
			found = true;
			resultsList << "Dead";
		}
	}
	else
	{
		if (searchRegExp.pattern ().toLower () == "alive")
		{
			found = true;
			resultsList << "Alive";
		}
	}

	if (_family && _family->getName ().contains (searchRegExp))
	{
		found = true;
		resultsList << QString ("Family Name: %1").arg (_family->getName ());
	}

	if (_village.contains (searchRegExp))
	{
		found = true;
		resultsList << QString ("Village: %1").arg (_village);
	}

	if (_address.contains (searchRegExp))
	{
		found = true;
		resultsList << QString ("Address: %1").arg (_address);
	}

	if (_imagePath.contains (searchRegExp))
	{
		found = true;
		resultsList << QString ("Image Path: %1").arg (_imagePath);
	}

	QString expandedHTMLNotes;
	QString expandedTextNotes = Utilities::convertFTMLToDisplayFormat (
				_notes, expandedHTMLNotes, true, false, -1,
				const_cast <Individual *>(this));
	if (expandedTextNotes.contains (searchRegExp))
	{
		found = true;
		resultsList << QString ("Notes:<br>%1<br>--- END ---").arg (expandedTextNotes);
	}

	return found;
}
