#include "Individual.h"

#define DEFINE(NAME,TITLE)	"<defn title=\"Alternately: " # TITLE "\">" # NAME "</defn>"
#define GETRELATIVESHELPERFUNCTION(R1A,R1B,R2A,R2B,FUNCTION) \
	allok = true; \
	individuals = FUNCTION; \
	if (individuals.size ()) \
	{ \
		ret += DEFINE (R1A and R2A, R1B and R2B) ": "; \
        std::sort (individuals.begin (), individuals.end (), \
			(bool (*) (const Individual *, const Individual *)) isYoungerThan); \
		ret += getListOfAnchors (individuals, true, true); \
		ret += ".<br>"; \
	} \
	if (!allok) listMayBeIncomplete = true;


QList <Individual*> Individual::getElderBrothers () const
{
	QList <Individual *> retList;
	for (int i = 0; i <= lastElderSibling(); i++)
	{
		Individual *sibling = getSibling (i);
		if (sibling->isFemale ())
			continue;
		retList << sibling;
	}
	return retList;
}

QList <Individual*> Individual::getYoungerBrothers () const
{
	QList <Individual *> retList;
	int youngerSiblingIndex = firstYoungerSibling();
	if (youngerSiblingIndex < 0)
		return retList;

	for (int i = youngerSiblingIndex; i < siblingCount() + 1; i++)
	{
		Individual *sibling = getSibling (i);
		if (sibling->isFemale ())
			continue;
		retList << sibling;
	}
	return retList;
}

QList <Individual*> Individual::getBrothers () const
{
	QList <Individual *> retList = getElderBrothers ();
	retList << getYoungerBrothers ();
	return retList;
}

QList <Individual*> Individual::getSisters () const
{
	QList <Individual *> retList;
	foreach (Individual *sibling, getSiblings ())
	{
		if (sibling->isMale ())
			continue;
		retList << sibling;
	}
	return retList;
}

QList <Individual*> Individual::getSiblings () const
{
	QList <Individual *> retList;

	for (int i = 0; i < siblingCount () + 1; i++)
	{
		Individual *sibling = getSibling (i);
		if (!sibling || sibling == this)
			continue;
		retList << sibling;
	}

	return retList;
}

QList <Individual*> Individual::getElderMaleCousins (bool &allok) const
{
	return getOlderOrYoungCousins (true, allok);
}

QList <Individual*> Individual::getYoungerMaleCousins (bool &allok) const
{
	return getOlderOrYoungCousins (false, allok);
}

QList <Individual*> Individual::getMaleCousins (bool &allok) const
{
	QList <Individual *> retList;
	foreach (Individual *cousin, getCousins (allok))
	{
		if (cousin->isFemale ())
			continue;
		retList << cousin;
	}
	return retList;
}

QList <Individual*> Individual::getFemaleCousins (bool &allok) const
{
	QList <Individual *> retList;
	foreach (Individual *cousin, getCousins (allok))
	{
		if (cousin->isMale ())
			continue;
		retList << cousin;
	}
	return retList;
}

QList <Individual*> Individual::getCousins (bool &allok) const
{
	return getChachereCousins (allok) + getFufereCousins (allok)
			+ getMamereCousins (allok) + getMausereCousins (allok);
}

QList <Individual*> Individual::getJijaBhabhis (bool &allok) const
{
	QList <Individual *> retList;
	foreach (Individual *sibling, getSiblings () + getCousins (allok))
		retList << sibling->getSpouses ();
	return retList;
}

QList <Individual*> Individual::getStepParents () const
{
	QList <Individual *> retList;
	Individual *father = getFather ();
	Individual *mother = getMother ();
	if (mother && mother->isNonDummy ())
	{
		for (int i = 0; i < mother->spouseCount (); i++)
		{
			Individual *stepFather = mother->getSpouse (i);
			if (stepFather == father)
				continue;
			retList << stepFather;
		}
	}
	if (father)
	{
		for (int i = 0; i < father->spouseCount (); i++)
		{
			Individual *stepMother = father->getSpouse (i);
			if (stepMother == mother)
				continue;
			retList << stepMother;
		}
	}
	return retList;
}

QList <Individual*> Individual::getHalfSiblngs () const
{
	QList <Individual *> retList;

	Individual *father = getFather ();
	Individual *mother = getMother ();
	IDTYPE fatherID = father ? father->getId () : 0;
	IDTYPE motherID = mother ? mother->getId () : 0;
	QList <Individual *> stepParents = getStepParents ();
	foreach (Individual *stepParent, stepParents)
	{
		IDTYPE spid = stepParent->isFemale () ? fatherID : motherID;
		Union *aUnion = stepParent->getUnionWithSpouseID (spid);
		foreach (IdableWithChildren *child, aUnion->getChildren ())
			retList << dynamic_cast<Individual *> (child);
	}

	return retList;
}

QList <Individual*> Individual::getDadaDadis (bool &allok) const
{
	Individual *father = getFather ();
	if (!father || !father->getFather ())
		return QList <Individual *> ();

	QList <Individual *> retList;
	getGrandFatherMothers (father, retList, allok);
	return retList;
}

QList <Individual*> Individual::getTauTais (bool &allok) const
{
	//Mother's jeth-jethani's may not be the same as tau-tais.
	Individual *father = getFather ();
	if (!father)
		return QList <Individual *> ();

	QList <Individual *> retList;
	foreach (Individual *brotherOrCousin, father->getElderBrothers ()
		 + father->getElderMaleCousins (allok))
		addIndividualAndSpouse (brotherOrCousin, retList);
	return retList;
}

QList <Individual*> Individual::getChachaChachis (bool &allok) const
{
	//Mother's devar-devarani's may not be the same as chacha-chachis.
	Individual *father = getFather ();
	if (!father)
		return QList <Individual *> ();

	QList <Individual *> retList;
	foreach (Individual *brotherOrCousin, father->getYoungerBrothers ()
		 + father->getYoungerMaleCousins (allok))
		addIndividualAndSpouse (brotherOrCousin, retList);
	return retList;
}

QList <Individual*> Individual::getChachereCousins (bool &allok) const
{
	Individual *father = getFather ();
	if (!father || !father->hasSibling ())
		return QList <Individual *> ();

	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = getTauTais (allok) + getChachaChachis (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addChildrenForUncle (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getFufaBuas (bool &allok) const
{
	// Mother's nanad-nandois may not be the same as bua-fufas.
	Individual *father = getFather ();
	if (!father)
		return QList <Individual *> ();

	QList <Individual *> retList;
	foreach (Individual *bua, father->getSisters () + father->getFemaleCousins (allok))
		addIndividualAndSpouse (bua, retList);
	return retList;
}

QList <Individual*> Individual::getFufereCousins (bool &allok) const
{
	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = getFufaBuas (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addChildrenForAunt (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getBhatijaBhatijis (bool &allok) const
{
	return getSiblingOffsprings (false, allok);
}

QList <Individual*> Individual::getBhanjaBhanjis (bool &allok) const
{
	return getSiblingOffsprings (true, allok);
}

QList <Individual*> Individual::getNanaNanis (bool &allok) const
{
	Individual *mother = getMother ();
	if (!mother || !mother->getMother ())
		return QList <Individual *> ();

	QList <Individual *> retList;
	getGrandFatherMothers (mother, retList, allok);
	return retList;
}

QList <Individual*> Individual::getMamaMamis (bool &allok) const
{
	Individual *mother = getMother ();
	if (!mother)
		return QList <Individual *> ();

	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = mother->getSiblings ()
			+ mother->getFemaleCousins (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addIndividualAndSpouseForMale (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getMamereCousins (bool &allok) const
{
	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = getMamaMamis (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addChildrenForUncle (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getMausaMausis (bool &allok) const
{
	Individual *mother = getMother ();
	if (!mother || !mother->hasSibling ())
		return QList <Individual *> ();

	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = mother->getSiblings ()
			+ mother->getFemaleCousins (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addIndividualAndSpouseForFemale (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getMausereCousins (bool &allok) const
{
	QList <Individual *> retList;
	QList <Individual *> uncleAuntList = getMausaMausis (allok);
	foreach (Individual *uncleAunt, uncleAuntList)
		addChildrenForAunt (uncleAunt, retList);
	return retList;
}

QList <Individual*> Individual::getSpouses () const
{
	QList <Individual *> retList;
	for (int i = 0; i < spouseCount (); i++)
	{
		Individual *spouse = getSpouse (i);
		if (spouse->isDummy ())
			continue;

		retList << spouse;
	}
	return retList;
}

QList <Individual*> Individual::getOffsprings () const
{
	QList <Individual *> retList;
	// We are not using getSpouses() here because it will not
	// return dummy spouses. And so we will miss out on those
	// offsprings.
	for (int i = 0; i < spouseCount (); i++)
	{
		Individual *spouse = getSpouse (i);
		IDTYPE spid = spouse->getId ();
		for (int j = 0; j < offspringCount (spid); j++)
			retList << getOffspring (j, spid);
	}
	return retList;
}

QList <Individual*> Individual::getDamadsAndBahus () const
{
	QList <Individual *> retList;
	foreach (Individual *offspring, getOffsprings ())
		retList << offspring->getSpouses ();
	return retList;
}

QList <Individual*> Individual::getPotaPotis () const
{
	return getGrandChildren (false);
}

QList <Individual*> Individual::getNaatiNaatins () const
{
	return getGrandChildren (true);
}

QString Individual::getRelatives (bool &listMayBeIncomplete) const
{
	bool allok;
	listMayBeIncomplete = false;
	QString ret = "";
	QList <Individual*> individuals; // This is used in the macros below.

	if (getFather ())
		ret += DEFINE (Father, Pitaji) ": " + getFather ()->getAnchor (0, true, true)
				+ "<br>";
	if (getMother () && getMother ()->isNonDummy ())
		ret += DEFINE (Mother, Maa) ": " + getMother ()->getAnchor (0, true, true)
				+ "<br>";

	GETRELATIVESHELPERFUNCTION (Broter, Bhai, Sister, Bahen, getSiblings ());
	GETRELATIVESHELPERFUNCTION (Step Father, Sautela Pita, Step Mother, Sauteli Maa, getStepParents ());
	GETRELATIVESHELPERFUNCTION (Step Brother, Sautela Bhai, Step Sister, Sautela Bhai, getHalfSiblngs ());
	GETRELATIVESHELPERFUNCTION (Dada, Grandfather, Dadi, Grandmother, getDadaDadis (allok));

	GETRELATIVESHELPERFUNCTION (Tau, Uncle, Tai, Aunt, getTauTais (allok));
	GETRELATIVESHELPERFUNCTION (Chacha, Uncle, Chachi, Aunt, getChachaChachis (allok));
	GETRELATIVESHELPERFUNCTION (Chachera Bhai, Cousin, Chacheri Bahen, Cousin, getChachereCousins (allok));
	GETRELATIVESHELPERFUNCTION (Fufa, Uncle, Bua, Aunt, getFufaBuas (allok));
	GETRELATIVESHELPERFUNCTION (Fufera Bhai, Cousin, Fuferi Bahen, Cousin, getFufereCousins (allok));

	GETRELATIVESHELPERFUNCTION (Nana, Grandfather, Nani, Grandmother, getNanaNanis (allok));

	GETRELATIVESHELPERFUNCTION (Mama, Uncle, Mami, Aunt, getMamaMamis (allok));
	GETRELATIVESHELPERFUNCTION (Mamera Bhai, Cousin, Mameri Bahen, Cousin, getMamereCousins (allok));

	GETRELATIVESHELPERFUNCTION (Mausa, Uncle, Mausi, Aunt, getMausaMausis (allok));
	GETRELATIVESHELPERFUNCTION (Mausera Bhai, Cousin, Mauseri Bahen, Cousin, getMausereCousins (allok));

	GETRELATIVESHELPERFUNCTION (Jija, Brother-in-law, Bhabhi, Sister-in-law, getJijaBhabhis (allok));
	GETRELATIVESHELPERFUNCTION (Bhatija, Nephew, Bhatiji, Niece, getBhatijaBhatijis (allok));
	GETRELATIVESHELPERFUNCTION (Bhanja, Nephew, Bhanji, Niece, getBhanjaBhanjis (allok));

	GETRELATIVESHELPERFUNCTION (Husband, Pati, Wife, Patni, getSpouses ());
	GETRELATIVESHELPERFUNCTION (Son, Beta, Daughter, Beti, getOffsprings ());
	GETRELATIVESHELPERFUNCTION (Son-in-law, Damad, Daughter-in-law, Bahu, getDamadsAndBahus ());
	GETRELATIVESHELPERFUNCTION (Pota, Grand-son, Poti, Grand-daughter, getPotaPotis ());
	GETRELATIVESHELPERFUNCTION (Naati, Grand-son, Naatin, Grand-daughter, getNaatiNaatins ());

	GETRELATIVESHELPERFUNCTION (Sala, Sister-in-law, Salhaj, Sister-in-law, getSalaSalhajs (allok));
	GETRELATIVESHELPERFUNCTION (Saadhu, Brother-in-law, Sali, Sister-in-law, getSaadhuSalis (allok));

	GETRELATIVESHELPERFUNCTION (Jeth, Brother-in-law, Jethani, Sister-in-law, getJethJethanis (allok));
	GETRELATIVESHELPERFUNCTION (Devar, Brother-in-law, Devarani, Sister-in-law, getDevarDevarani (allok));
	GETRELATIVESHELPERFUNCTION (Nanadoi, Brother-in-law, Nanad, Sister-in-law, getNanadNandoi (allok));

	return ret;
}

void Individual::getGrandFatherMothers (const Individual *fatherOrMother,
					QList<Individual *> &retList, bool &allok) const
{
	Individual *grandFather = fatherOrMother->getFather ();
	if (grandFather)
		addIndividualAndSpouse (grandFather, retList);
	retList << fatherOrMother->getTauTais (allok);
	retList << fatherOrMother->getChachaChachis (allok);
	retList << fatherOrMother->getFufaBuas (allok);
}

QList <Individual*> Individual::getSiblingOffsprings (bool female, bool &allok) const
{
	QList <Individual *> retList;
	foreach (Individual *sibling, getSiblings () + getCousins (allok))
		addChildrenForUncleOrAunt (sibling, retList, !female);
	return retList;
}

QList <Individual*> Individual::getGrandChildren (bool female) const
{
	QList <Individual *> retList;
	foreach (Individual *offspring, getOffsprings ())
	{
		if (offspring->isMale ()== female)
			continue;

		retList << offspring->getOffsprings ();
	}
	return retList;
}

QList <Individual*> Individual::getOlderOrYoungCousins (bool older, bool &allok) const
{
	QList <Individual *> retList;
	allok = true;
	foreach (Individual *cousin, getMaleCousins (allok))
	{
		try
		{
			if (isOlderThan (cousin) == older)
				retList << cousin;
		}
        catch (NoBirthEventException &noBirthEventException)
		{
			// Adding to Devar list in case birth date is missing.
			if (!older)
				retList << cousin;
			allok = false;
		}
	}
	return retList;

}

QString Individual::getRelativesAsDisplayString () const
{
	bool listMayBeIncomplete = true;
	QString ret = getRelatives (listMayBeIncomplete);

	if (listMayBeIncomplete)
		ret += "The list above might be inaccurate to some extent, since some of the cousins have missing birth dates.<br>";
	if (ret.size ())
		ret = "<h3>List of Relatives</h3>" + ret;

	return ret;
}

void Individual::getRelativesHelperFunction (const QString maleRelation,
					     const QString femaleRelation,
					     const QList <Individual *> relatives,
					     QList<QPair<QString, Individual *> > &relationList) const
{
	foreach (Individual *individual, relatives)
	{
		if (individual->isMale ())
			relationList << QPair<QString, Individual*> (maleRelation, individual);
		else
			relationList << QPair<QString, Individual*> (femaleRelation, individual);
	}
}

void Individual::addIndividualAndSpouse (const Individual *individual,
					 QList<Individual *> &retList) const
{
	retList << const_cast<Individual*>(individual);
	for (int i = 0; i < individual->spouseCount (); i++)
	{
		Individual *spouse = individual->getSpouse (i);
		if (spouse->isDummy ())
			continue;
		retList << spouse;
	}
}

void Individual::addIndividualAndSpouseForMale (const Individual *individual,
						QList <Individual *> &retList) const
{
	if (individual->isFemale ())
		return;

	addIndividualAndSpouse (individual, retList);
}

void Individual::addIndividualAndSpouseForFemale (const Individual *individual,
						  QList<Individual *> &retList) const
{
	if (individual->isMale ())
		return;

	addIndividualAndSpouse (individual, retList);
}

void Individual::addChildrenForUncleOrAunt (const Individual *uncleAunt,
					    QList <Individual *> &retList,
					    bool forUncle) const
{
	if (uncleAunt->isFemale () == forUncle)
		return;

	retList << uncleAunt->getOffsprings ();
}

void Individual::addChildrenForUncle (const Individual *uncleAunt,
				      QList<Individual *> &retList) const
{
	addChildrenForUncleOrAunt (uncleAunt, retList, true);
}

void Individual::addChildrenForAunt (const Individual *uncleAunt,
				     QList<Individual *> &retList) const
{
	addChildrenForUncleOrAunt (uncleAunt, retList, false);
}
