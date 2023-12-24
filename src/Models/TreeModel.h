#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "TreeItem.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QMimeData>

#include "Family.h"
#include "Individual.h"

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	enum t_columns {
		e_col_name,
		e_col_link,			// Individual
		e_col_alias,
		e_col_sex,			// Individual
		e_col_dead,			// Individual
		e_col_birthDate,		// Individual
		e_col_marriageDate,		// Individual
		e_col_deathDate,		// Individual
		e_col_facebookID,		// Individual
		e_col_dummyOrBase,		// Individual & Family

		// Entries beyond this are not shown by default by the model
		e_col_basicMaximum,		// None

		e_col_male,
		e_col_female,
		e_col_facebookURL,
		e_col_myGeneration,		// Individual
		e_col_isIndividual,
		e_col_individualIsDummy,
		e_col_isFamily,
		e_col_familyIsBase,
		e_col_hasBirthEvent,
		e_col_hasDeathEvent,
		e_col_hasMarriageEvent,
		e_col_sonCount,
		e_col_daughterCount,
		e_col_marriedDaughterCount,
		e_col_spouseCount,
		e_col_aliveCount,
		e_col_deadCount,
		e_col_totalCount,
		e_col_generations,
		e_col_icon,			// Individual

		// Data for the following entries is generated from this model itself
		e_col_uniqueIdentifier,
		e_col_expandable,
		e_col_ftName,
		e_col_ftBirthDate,
		e_col_ftMarriageDate,
		e_col_ftDeathDate,
		e_col_isSpouseItem,
		e_col_childCount,

		e_col_maximum			// None
	};

private:
    QHash <int, Familiar::t_columns> mHash_ColumnToColumnMapping;
	TreeItem *			_rootItem;

private:
	static const QString		s_mimetype;

public:
	TreeModel					(QObject *parent = 0);
	TreeModel					(TreeItem *rootItem, QObject *parent = 0);
	~TreeModel					();

private:
	void			Initialize		();

public:
	TreeItem *		getItem			(const QModelIndex &index) const;
	TreeItem *		getChild		(const QModelIndex &index, const int row) const;

	virtual QModelIndex	index			(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex		index			(TreeItem *item, int column = 0) const;

	virtual QModelIndex	parent			(const QModelIndex &index) const;
	QModelIndex		parent			(TreeItem *childItem) const;

	virtual bool		hasChildren		(const QModelIndex &parent = QModelIndex()) const;
	virtual int		rowCount		(const QModelIndex &parent = QModelIndex()) const;
	virtual int		columnCount		(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant	headerData		(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual Qt::ItemFlags	flags			(const QModelIndex &index) const;
	virtual QVariant	data			(const QModelIndex &index, int role) const;
	virtual bool		setData			(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    bool			insertRow		(Familiar *fam, bool createItemOnly, int row,
							 const QModelIndex &parent = QModelIndex(),
							 IDTYPE uid = 0);
	virtual bool		removeRow		(int row, const QModelIndex &parent = QModelIndex());

	Qt::DropActions		supportedDragActions	() const;
	virtual Qt::DropActions	supportedDropActions	() const;

	virtual QStringList	mimeTypes		() const;
	virtual QMimeData*	mimeData		(const QModelIndexList &indexes) const;
	virtual bool		dropMimeData		(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

	void			clear			();

private:
    Familiar::t_columns	toFamiliarColumn	(const int treeModelColumn) const;
	QVariant		unchangedData		(const QModelIndex &index, int role) const;

private slots:
	void			adjustForModelReset	();
	void			adjustForFamilyNameChange ();

signals:
	void			rowInserted		();
	void			itemDumminessChanged	(TreeItem *);
	void			newEventAdded		();
	void			expandAssociatedTree	();
};

#endif /* TREEMODEL_H */
