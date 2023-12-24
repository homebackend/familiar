#include "Individual.h"
#include "SettingsManager.h"

QTextStream& operator << (QTextStream &stream, Individual &individual)
{
	stream << "\t<Individual ";
	stream << dynamic_cast <Idable &> (individual);
	if (individual.isFemale())
		stream << " type=\"Female\"";
	if (individual._dummy)
		stream << " dummy=\"1\"";
	if (individual._dead)
		stream << " dead=\"1\"";
	if (!individual._isVillageInherited)
	{
		QString village = individual._village;
		Idable::escapeStringForXML (village);
		stream << " village=\"" << village << "\"";
	}
	if (!individual._isAddressInherited)
	{
		QString address = individual._address;
		Idable::escapeStringForXML (address);
		stream << " address=\"" << address << "\"";
	}
	if (individual._imagePath.length())
	{
		QString imagePath = individual._imagePath;
		Idable::escapeStringForXML (imagePath);
		stream << " imagePath=\"" << imagePath << "\"";
	}
	if (individual._facebookID.length ())
	{
		QString facebookID = individual._facebookID;
		Idable::escapeStringForXML (facebookID);
		stream << " facebookID=\"" << facebookID << "\"";
	}
	if (individual._adoptedFrom)
		stream << " adoptedFrom=\"" << individual._adoptedFrom->getId() << "\"";
	if (individual._adoptedTo)
		stream << " adoptedTo=\"" << individual._adoptedTo->getId() << "\"";
	if (individual._birthEvent)
		stream << " birthEvent=\"" << individual._birthEvent->getId() << "\"";
	if (individual._deathEvent)
		stream << " deathEvent=\"" << individual._deathEvent->getId() << "\"";
    stream	<< ">" << Qt::endl;

    stream << dynamic_cast <Familiar &> (individual);
	if (individual._notes.length())
	{
		QString notes = individual._notes;
		Idable::escapeStringForXML(notes);
        stream << "\t\t<Notes>" << Qt::endl
                << notes << Qt::endl
                << "\t\t</Notes>" << Qt::endl;
	}
    stream << "\t</Individual>" << Qt::endl;
	return stream;
}

QTextStream& Individual::toGEDCOM(QTextStream &stream) const
{
	if (isDummy ())
		return stream;

    stream << "0 @I" << _id << "@ INDI" << Qt::endl;

	stream << "1 NAME " << _name;
	if (SettingsManager::s_ExportForceLastName
			|| (getFamily () && getFamily ()->isChild (_id)))
		stream << " /" << getSurName () << "/";
    stream << Qt::endl;
    stream << "1 SEX " << (isFemale() ? "F" : "M") << Qt::endl;

	QString noteString;
	QTextStream noteStream (&noteString);

	if (_birthEvent)
	{
        stream << "1 BIRT" << Qt::endl;
		_birthEvent->toGEDCOM (stream);

		if (_birthEvent->hasDescription ())
			notesToGEDCOM (_birthEvent->getDescription (), noteStream);
	}
	if (_deathEvent)
	{
        stream << "1 DEAT" << Qt::endl;
		_deathEvent->toGEDCOM (stream);

		if (_deathEvent->hasDescription ())
			notesToGEDCOM (_deathEvent->getDescription (), noteStream);
	}
	if (_parentUnion)
        stream << "1 FAMC @F" << _parentUnion->getId () << "@" << Qt::endl;
	foreach (IdableWithChildren *aUnion, _children)
        stream << "1 FAMS @F" << aUnion->getId() << "@" << Qt::endl;
	if (_aliases.length())
        stream << "1 ALIA " << _aliases.at(0) << Qt::endl;
	if (_address.length())
        stream << "1 ADDR " << _address << Qt::endl;
    stream << "1 CHAN" << Qt::endl;
    stream << "2 DATE " << getModificationTime().toString("dd MMM yyyy") << Qt::endl;
    stream << "3 TIME " << getModificationTime ().toString("hh:mm:ss") << Qt::endl;
	if (_notes.length())
	{
        stream << "1 NOTE @N" << ++ IdableWithChildren::s_commonNotesID << "@" << Qt::endl;
		notesToGEDCOM (_notes, noteStream);
	}

	stream << noteString;

	return stream;
}

void Individual::fromGEDCOM (const QStringList &gedcomEntryStringList,
			     const QHash<QString, Union*> &xRefToUnionHash,
			     const QHash <QString, QStringList> &xRefToNoteGEDCOMInfoHash)
{
	blockRecordingOfItemModifications ();

	QStringList eventGedComEntryStringList;
	enum { none, birth, death } eventMode = none;

	foreach (QString line, gedcomEntryStringList)
	{
		QList<QString> list = line.split (QRegExp ("\\s+"));
		if (list.count () <= 1)
			continue;

		int level = list.at (0).toInt ();

		switch (level)
		{
		case 1:
			if (eventMode != none)
			{
				switch (eventMode)
				{
				case birth:
					{
						Event *event = new Event;
						event->setName ("Birth");
						event->appendChild (this);
						if (getBirthEvent ())
							delete getBirthEvent ();
						setBirthEvent (event);
						event->fromGEDCOM (eventGedComEntryStringList,
								   xRefToNoteGEDCOMInfoHash);
					}
					break;
				case death:
					{
						Event *event = new Event;
						event->setName ("Death");
						event->appendChild (this);
						if (getDeathEvent ())
							delete getDeathEvent ();
						setDeathEvent (event);
						event->fromGEDCOM (eventGedComEntryStringList,
								   xRefToNoteGEDCOMInfoHash);
					}
					break;
				default:
					break;
				}
				eventMode = none;
				eventGedComEntryStringList.clear ();
			}
			{
				QString tag = list.at (1);
				if (tag == "SEX")
					continue;
				else if (tag == "ALIA")
				{
					if (list.size () < 3)
						continue;
					_aliases << list.at (2);
				}
				else if (tag == "NAME")
				{
					if (list.size () < 3)
						continue;

					QString name = list.at (2);
					for (int i = 3; i < list.size (); i++)
						name += " " + list.at (i);

					QStringList names = name.split ('/');
					if (names.count () == 1)
						setName (name);
					else
					{
						// firstName includes all except last name
						QString firstName = names.at (0).trimmed ();
						QString lastName = names.at (1).trimmed ();
						setName (firstName);

						if (!lastName.isEmpty ())
						{
							Family *family;
							if (!Family::familyExists (lastName))
							{
								family = new Family;
								family->setName (lastName);
							}
							else
								family = Family::getFamilyWithName (lastName);
							setFamily (family);
							family->appendChild (this);
						}
					}
				}
				else if (tag == "FAMC")
				{
					if (list.size () < 3)
						continue;

					QString fxRef = list.at (2);
					Union *aUnion = xRefToUnionHash.value (fxRef, NULL);
					if (aUnion)
					{
						if (getParentUnion ())
							delete getParentUnion ();
						setParentUnion (aUnion);
					}
				}
				else if (tag == "FAMS")
				{
					if (list.size () < 3)
						continue;

					QString fxRef = list.at (2);
					Union *aUnion = xRefToUnionHash.value (fxRef, NULL);
					if (aUnion)
						appendChild (aUnion);
				}
				else if (tag == "NOTE")
				{
					if (list.size () < 3)
						continue;
					QString fxRef = list.at (2);
					const QStringList &noteGedComList = xRefToNoteGEDCOMInfoHash.value (fxRef);
					_notes = IdableWithChildren::notesFromGEDCOM (noteGedComList);
				}
				else if (tag == "BIRT")
					eventMode = birth;
				else if (tag == "DEAT")
					eventMode = death;
			}
			break;
		case 2:
		case 3:
			{
				switch (eventMode)
				{
				case birth:
				case death:
					eventGedComEntryStringList << line;
					break;
				default:
					break;
				}
			}
			break;
		default:
			break;
		}
	}

	unblockRecordingOfItemModifications ();
}

QDebug operator << (QDebug debug, Individual &individual)
{
    debug << "Individual Start" << Qt::endl;
	debug << dynamic_cast <Idable &> (individual);
    debug << "Type:" << (individual.isFemale() ? "Female" : "Male") << Qt::endl;
    debug << "Dummy:" << individual._dummy << Qt::endl;
    debug << "Dead:" << individual._dead << Qt::endl;
    debug << "Father:" << (individual.getFather () ? individual.getFather ()->getId () : 0) << Qt::endl;
    debug << "Mother:" << (individual.getMother () ? individual.getMother ()->getId () : 0) << Qt::endl;
    debug << "Adopted From:" << (individual._adoptedFrom ? individual._adoptedFrom->getId() : 0) << Qt::endl;
    debug << "Adopted To:" << (individual._adoptedTo ? individual._adoptedTo->getId() : 0) << Qt::endl;
    debug << "Birth Event:" << (individual._birthEvent ? individual._birthEvent->getId() : 0) << Qt::endl;
    debug << "Death Event:" << (individual._deathEvent ? individual._deathEvent->getId() : 0) << Qt::endl;
    debug << "Family:" << (individual._family ? individual._family->getId() : 0) << Qt::endl;
    debug << "Village:" << individual._village << Qt::endl;
    debug << "Address:" << individual._address << Qt::endl;
    debug << "Image Path:" << individual._imagePath << Qt::endl;
    debug << dynamic_cast <Familiar &> (individual);
    debug << "Notes Start" << Qt::endl;
    debug << individual._notes << Qt::endl;
    debug << "Noted End" << Qt::endl;
    debug << "Individual End" << Qt::endl;
    debug << Qt::endl;
	return debug;
}
