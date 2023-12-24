#ifndef CLASSTREEITEM_H
#define CLASSTREEITEM_H

#include "Familiar.h"
#include <QHash>
#include <QList>
#include <QUuid>

class TreeItem
{
	/* List automatically maintains a list of all tree items.
	 * This will be useful for List and Icon view models.
	 */
public:
	static QList<TreeItem *> familyTreeItemList;
	static QHash<IDTYPE, TreeItem*> idToFamilyTreeItemHash;
	static QList<TreeItem *> individualTreeItemList;
	static QHash<IDTYPE, TreeItem*> idToIndividualTreeItemHash;
	static TreeItem * s_RootItemForFamilyTreeView;

private:
	QUuid			_uuid;
	bool			_isExpandable;
	bool			_copyConstructed;
    Familiar *		_fam;
	TreeItem *		_parentItem;
	QList <TreeItem *>	_childItems;

	bool			_isIndividual;
	bool			_isSpouseItem;

public:
    TreeItem (Familiar *fam, TreeItem *parentItem);
	TreeItem (const TreeItem &treeItem);
	~TreeItem ();

    TreeItem&       operator =      (const TreeItem &) = default;

    Familiar *	fam			() const { return _fam; }
	TreeItem *	parentItem		() const { return _parentItem; }
	TreeItem *	leftMostChild		() const;
	TreeItem *	rightMostChild		() const;
	TreeItem *	leftChildFrom		(const TreeItem *refChild) const;
	TreeItem *	rightChildFrom		(const TreeItem *refChild) const;

	QUuid		uuid			() const { return _uuid; }
	IDTYPE		id			() const;
	bool		isExpandable		() const { return _isExpandable; }
	bool		isRoot			() const { return _parentItem == NULL; }
	bool		isIndividual		() const { return _isIndividual; }
	bool		isFemale		() const;
	bool		isSpouseItem		() const { return _isSpouseItem; }
	IDTYPE		spouseID		() const;
	bool		isDead			() const;
	bool		isDummy			() const;
	bool		hasImage		() const;

	int		myChildNumber		() const;

	bool		hasChild		() const;
	int		childCount		() const;
	bool		appendChild		(TreeItem *child);
	bool		removeChild		(TreeItem *child);
	bool		insertChild		(const long chidx, TreeItem *child);
	void		replaceChild		(int chidx, TreeItem *child);
	TreeItem*	childAt			(const long chidx) const;
	int		indexOfChild		(const TreeItem *child) const;
	bool		setExpandable		(const bool isExpandable);

	void		clear			();
};

bool			operator <		(const TreeItem &item1, const TreeItem &item2);
bool			operator >		(const TreeItem &item1, const TreeItem &item2);
bool			operator ==		(const TreeItem &item1, const TreeItem &item2);

#endif // CLASSTREEITEM_H
