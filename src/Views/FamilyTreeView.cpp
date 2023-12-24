#include "FamilyTreeView.h"
#include "TreeModel.h"
#include "Utilities.h"
#include <QApplication>
#include <QScrollBar>
#include <QFontMetrics>
#include <QFont>
#include <qmath.h>

#ifndef M_PI
#define M_PI 3.1415927
#endif

FamilyTreeView::FamilyTreeView(QWidget *parent) :
		QAbstractItemView(parent),
		_idealHeight (0), _idealWidth (0), _hashNeedsUpdate (false),
		_showExtraDetailsForItem (true), _showBasicDetailsForItem (true),
		_xStep (2), _yStep (4),
		_plusLength (4), _plusPadding (2),
		_itemSeparation (10), _lineSeparation (2), _iconSize (64), _dummyDotSize (8)
{
	setFocusPolicy(Qt::WheelFocus);
	setFont(QApplication::font("QTreeView"));
	horizontalScrollBar()->setRange(0, 0);
	verticalScrollBar()->setRange(0, 0);
	setDragEnabled (true);
	setAcceptDrops (true);
	setDropIndicatorShown (true);
	setTabKeyNavigation (true);
	setSelectionMode (QAbstractItemView::SingleSelection);
}

void FamilyTreeView::setModel (QAbstractItemModel *model)
{
	QAbstractItemView::setModel (model);
	_hashNeedsUpdate = true;
}

QRect FamilyTreeView::visualRect (const QModelIndex &index) const
{
	QRect rect;
	if (index.isValid ())
		rect = viewportRectForIndex (index).toRect();
	return rect;
}

void FamilyTreeView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint scrollHint)
{
	if (!index.isValid ())
		return;

	QRect viewRect = viewport()->rect();
	QRect itemRect = viewportRectForItem (index).toRect ();

	if (itemRect.left() < viewRect.left())
		horizontalScrollBar()->setValue(horizontalScrollBar()->value()
						+ itemRect.left() - viewRect.left());
	else if (itemRect.right() > viewRect.right())
		horizontalScrollBar()->setValue(horizontalScrollBar()->value()
						+ qMin(itemRect.right() - viewRect.right(),
						       itemRect.left() - viewRect.left()));
	if (itemRect.top() < viewRect.top())
		verticalScrollBar()->setValue(verticalScrollBar()->value() +
					      itemRect.top() - viewRect.top());
	else if (itemRect.bottom() > viewRect.bottom())
		verticalScrollBar()->setValue(verticalScrollBar()->value() +
					      qMin(itemRect.bottom() - viewRect.bottom(),
						   itemRect.top() - viewRect.top()));

	itemRect = viewportRectForItem (index).toRect ();
	viewRect = viewport ()->rect ();
	switch (scrollHint)
	{
	case QAbstractItemView::PositionAtTop:
		if (itemRect.top () > 0)
			verticalScrollBar ()->setValue (verticalScrollBar ()->value () +
							viewRect.top () - itemRect.top ());
		break;
	case QAbstractItemView::PositionAtBottom:
		if (itemRect.top () < viewRect.bottom ())
			verticalScrollBar ()->setValue (verticalScrollBar ()->value () -
							viewRect.bottom () + itemRect.top ());
		break;
	case QAbstractItemView::PositionAtCenter:
		{
			int xViewMid = viewRect.width ()/ 2;
			int yViewMid = viewRect.height ()/ 2;
			if (itemRect.left () < xViewMid)
				horizontalScrollBar()->setValue(horizontalScrollBar()->value()-
								xViewMid + itemRect.width () / 2);
			else
				horizontalScrollBar()->setValue(horizontalScrollBar()->value() +
								xViewMid - itemRect.width () / 2);
			if (itemRect.top () < yViewMid)
				verticalScrollBar()->setValue(verticalScrollBar()->value() -
							      yViewMid + itemRect.height () / 2);
			else
				verticalScrollBar()->setValue(verticalScrollBar()->value() +
							      yViewMid + itemRect.height () / 2);
		}
		break;
	default:
		break;
	}

	viewport()->update();
}

QModelIndex FamilyTreeView::indexAt (const QPoint &point) const
{
	QPoint relPoint (point);
	relPoint.rx() += horizontalScrollBar()->value();
	relPoint.ry() += verticalScrollBar()->value();

	calculateRectsIfRequired ();
	QModelIndex rootIdx = rootIndex ();
	return indexAtRecursive (relPoint, rootIdx);
}

bool FamilyTreeView::getShowExtraDetailsForItem () const
{
	return _showExtraDetailsForItem;
}

bool FamilyTreeView::getShowBasicDetailsForItem () const
{
	return _showBasicDetailsForItem;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void FamilyTreeView::dataChanged(const QModelIndex &topLeft,
                                 const QModelIndex &bottomRight,
                                 const QVector<int> &roles)
#else
void FamilyTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
#endif
{
	_hashNeedsUpdate = true;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
#else
    QAbstractItemView::dataChanged(topLeft, bottomRight);
#endif
}

void FamilyTreeView::rowsInserted (const QModelIndex &parent, int start, int end)
{
	_hashNeedsUpdate = true;
	QAbstractItemView::rowsInserted (parent, start, end);
}

void FamilyTreeView::rowsAboutToBeRemoved (const QModelIndex &parent, int start, int end)
{
	_hashNeedsUpdate = true;
	QAbstractItemView::rowsAboutToBeRemoved (parent, start, end);
}

void FamilyTreeView::updateGeometries()
{
	QFontMetrics fm(font());
    horizontalScrollBar()->setSingleStep(Utilities::horizontalAdvance(fm, "n"));
	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setRange(0, qMax(0.0, _idealWidth - viewport()->width()));
	const int RowHeight = fm.height();
	verticalScrollBar()->setSingleStep(RowHeight);
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setRange(0, qMax(0.0, _idealHeight - viewport()->height()));
}

bool FamilyTreeView::edit (const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    if (model()->flags(index).testFlag(Qt::ItemIsEditable))
		return QAbstractItemView::edit (index, trigger, event);

	return false;
}

QModelIndex FamilyTreeView::moveCursor (QAbstractItemView::CursorAction cursorAction,
					Qt::KeyboardModifiers modifiers)
{
	QModelIndex index = currentIndex ();
	if (index.isValid ())
	{
		QModelIndex parent = index.parent ();
		QAbstractItemModel *tm = model ();
		QModelIndex newIndex;
		switch (cursorAction)
		{
		case MoveLeft:
			if (modifiers == Qt::NoModifier && parent.isValid ())
				newIndex = leftChildFrom (parent, index);
			if (!newIndex.isValid ())
				horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () - 10);
			break;
		case MoveRight:
			if (modifiers == Qt::NoModifier && parent.isValid ())
				newIndex = rightChildFrom (parent, index);
			if (!newIndex.isValid ())
				horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () + 10);
			break;
		case MoveUp:
			if (modifiers == Qt::NoModifier)
			{
				newIndex = parent;
				bool isDummy = model ()->index (newIndex.row (), TreeModel::e_col_individualIsDummy, newIndex.parent ()).data ().toBool ();
				while (newIndex.isValid () && isDummy && newIndex.parent ().isValid ())
					newIndex = newIndex.parent ();
			}
			if (!newIndex.isValid ())
				verticalScrollBar ()->setValue (verticalScrollBar ()->value () - 10);
			break;
		case MoveDown:
			if (modifiers == Qt::NoModifier && tm->hasChildren (index))
			{
				QModelIndex tempIndex = index;
				do
				{
					if (tm->hasChildren (index))
						newIndex = leftMostChild (index);
					tempIndex = newIndex;
				} while (tm->index (newIndex.row (), TreeModel::e_col_individualIsDummy,
						    newIndex.parent ()).data ().toBool ()
					 && tm->hasChildren (newIndex));
			}
			else
				verticalScrollBar ()->setValue (verticalScrollBar ()->value () + 10);
			break;
		default:
			break;
		}

		if (newIndex.isValid ())
			index = newIndex;
	}

	viewport()->update();
	return index;
}

int FamilyTreeView::horizontalOffset() const
{
	return horizontalScrollBar()->value();
}

int FamilyTreeView::verticalOffset() const
{
	return verticalScrollBar()->value();
}

void FamilyTreeView::scrollContentsBy(int dx, int dy)
{
	scrollDirtyRegion(dx, dy);
	viewport()->scroll(dx, dy);
}

bool FamilyTreeView::isIndexHidden (const QModelIndex &index) const
{
	Q_UNUSED(index)
	return false;
}

void FamilyTreeView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
	QRect rectangle = rect.translated(horizontalScrollBar()->value(),
					  verticalScrollBar()->value()).normalized();

	calculateRectsIfRequired ();
	QModelIndex rootIdx = rootIndex ();

	if (!setSelectionRecursively (rootIdx, rectangle, flags))
	{
		QModelIndex invalid;
		QItemSelection selection(invalid, invalid);
		selectionModel()->select(selection, flags);
	}

	update();
}

void FamilyTreeView::mousePressEvent(QMouseEvent *event)
{
	QAbstractItemView::mousePressEvent(event);
	QModelIndex idx = indexAt (event->pos());
	if (idx.isValid ())
		setCurrentIndex (idx);
	else
	{
		QPoint p = event->pos ();
		p.ry () -= (_yStep + 2 * _plusLength + _plusPadding);
		QModelIndex index = indexAt (p);
		if (index.isValid () && !isExpanded (index))
		{
			QString uuid = getStringUuid (index);
			QRect rect = viewportRectForItemTerminator (uuid).toRect ();
			if (rect.contains (event->pos ()))
				expand (index);
		}
	}
}

void FamilyTreeView::mouseDoubleClickEvent (QMouseEvent *event)
{
	QModelIndex index = indexAt (event->pos ());
	if (index.isValid ())
		setExpanded (index, !isExpanded (index));

	QAbstractItemView::mouseDoubleClickEvent (event);
}

void FamilyTreeView::keyPressEvent (QKeyEvent *event)
{
	int key = event->key ();
	switch (key)
	{
	case Qt::Key_Asterisk:
		expandAll ();
		break;
	case Qt::Key_Period:
		collapseAll ();
		break;
	case Qt::Key_Plus:
		{
			QModelIndex currentIdx = currentIndex ();
			if (currentIdx.isValid ())
				expand (currentIdx);
		}
		break;
	case Qt::Key_Minus:
		{
			QModelIndex currentIdx = currentIndex ();
			if (currentIdx.isValid ())
				collapse (currentIdx);
		}
		break;
	case Qt::Key_Up:
		if (!currentIndex ().isValid ())
			verticalScrollBar ()->setValue (verticalScrollBar ()->value () - 10);
		break;
	case Qt::Key_Down:
		if (!currentIndex ().isValid ())
			verticalScrollBar ()->setValue (verticalScrollBar ()->value () + 10);
		break;
	case Qt::Key_PageUp:
		verticalScrollBar ()->setValue (verticalScrollBar ()->value () -
						viewport ()->rect ().height ());
		break;
	case Qt::Key_PageDown:
		verticalScrollBar ()->setValue (verticalScrollBar ()->value () +
						viewport ()->rect ().height ());
		break;
	case Qt::Key_Home:
		verticalScrollBar ()->setValue (0);
		break;
	case Qt::Key_End:
		verticalScrollBar ()->setValue (verticalScrollBar ()->maximum ());
		break;
	case Qt::Key_Left:
		if (!currentIndex ().isValid ())
			horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () - 10);
		break;
	case Qt::Key_Right:
		if (!currentIndex ().isValid ())
			horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () + 10);
		break;
	case Qt::Key_BracketLeft:
		horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () -
						  viewport ()->rect ().width ());
		break;
	case Qt::Key_BracketRight:
		horizontalScrollBar ()->setValue (horizontalScrollBar ()->value () +
						  viewport ()->rect ().width ());
		break;
	case Qt::Key_BraceLeft:
		horizontalScrollBar ()->setValue (0);
		break;
	case Qt::Key_BraceRight:
		horizontalScrollBar ()->setValue (horizontalScrollBar ()->maximum ());
		break;
	default:
		break;
	}

	QAbstractItemView::keyPressEvent (event);
}

void FamilyTreeView::paintEvent (QPaintEvent *event)
{
	Q_UNUSED(event)

	calculateRectsIfRequired ();
	updateGeometries ();

	QPainter painter (viewport());
	painter.setRenderHints (QPainter::Antialiasing | QPainter::TextAntialiasing);

	paintIndexesRecursively (rootIndex (), painter);
}

void FamilyTreeView::resizeEvent(QResizeEvent*)
{
	_hashNeedsUpdate = true;
	calculateRectsIfRequired ();
	updateGeometries();
}

QRegion FamilyTreeView::visualRegionForSelection (const QItemSelection &selection) const
{
	QRegion region;
	foreach (const QItemSelectionRange &range, selection)
	{
		for (int row = range.top(); row <= range.bottom(); ++row)
		{
			for (int column = range.left(); column < range.right(); ++column)
			{
				QModelIndex index = model()->index(row, column, rootIndex());
				region += visualRect(index);
			}
		}
	}

	return region;
}

QRectF FamilyTreeView::viewportRectForIndex (const QModelIndex &index) const
{
	if (!index.isValid ())
		return QRectF ();

	calculateRectsIfRequired ();

	QRectF rect = _rectForIndex.value (index);
	if (!rect.isValid ())
		return rect;
	return QRectF (rect.x() - horizontalScrollBar()->value(),
		       rect.y() - verticalScrollBar()->value(),
		       rect.width(), rect.height());
}

QRectF FamilyTreeView::viewportRectForItem (const QString &uuid) const
{
	calculateRectsIfRequired ();

	QRectF rect = _rectForItem.value (uuid);
	if (!rect.isValid ())
		return rect;

	return QRectF (rect.x() - horizontalScrollBar()->value(),
		       rect.y() - verticalScrollBar()->value(),
		       rect.width(), rect.height());
}

QRectF FamilyTreeView::viewportRectForItem (const QModelIndex &index) const
{
	QString uuid = getStringUuid (index);
	return viewportRectForItem (uuid);
}

QRectF FamilyTreeView::viewportRectForItemTerminator (const QString &uuid) const
{
	QRectF rect = viewportRectForItem (uuid);
	if (!rect.isValid ())
		return rect;

	qreal xmid = (rect.left () + rect.right ()) / 2;
	qreal rectSize = 2 * _itemSeparation;
	return QRectF (xmid - _itemSeparation, rect.bottom (), rectSize, rectSize);
}

qreal FamilyTreeView::setRectForField (const QModelIndex &index, qreal &y) const
{
	const int horizontalPadding = 50;

	qreal pixmapWidth = 0;
	qreal pixmapHeight = 0;
	QVariant value = index.data (Qt::DecorationRole);
	if (value.canConvert<QPixmap> ())
	{
		QPixmap pixmap = value.value<QPixmap>();
		pixmapWidth = pixmap.width ();
		pixmapHeight = pixmap.height ();
	}

	value = index.data (Qt::FontRole);
	QFont fx;
	if (value.canConvert<QFont>())
		fx = value.value<QFont>();
	else
		fx = font ();
	QFontMetrics fmx (fx);
	y += _lineSeparation;
	qreal hx = qMax (qreal (fmx.height ()), pixmapHeight);
    const auto strData = index.data (Qt::DisplayRole).toString ();
    qreal wx = Utilities::horizontalAdvance(fmx, strData) + pixmapWidth + horizontalPadding;
	_rectForIndex [index] = QRectF (0, y, wx, hx);
	y += hx;

	return wx;
}

void FamilyTreeView::calculateRectsIfRequired () const
{
	if (!_hashNeedsUpdate)
		return;

	_idealHeight = 0;
	_idealWidth = 0;

	_rectForIndex.clear ();
	_rectForItem.clear ();

	QModelIndex index = rootIndex ();
	if (index.isValid ())
		calculateRectsIfRequired (index);
	else
	{
		QAbstractItemModel *tm = model ();
		QString uuid = "RootItem";
		_rectForItem [uuid] = QRectF (0, 0, 0, 0);
		for (int i = 0; i < tm->rowCount (QModelIndex ()); i++)
		{
			calculateRectsIfRequired (tm->index (i, TreeModel::e_col_name, QModelIndex ()));
			_rectForItem [uuid] = QRectF (0, _idealHeight, 0, 0);
		}
		_rectForItem.remove (uuid);
	}

	_hashNeedsUpdate = false;
	viewport ()->update ();
}

void FamilyTreeView::calculateRectsIfRequired (const QModelIndex &currentIndex) const
{
	int row = currentIndex.row ();

	QModelIndex parentIndex = currentIndex.parent ();
	QAbstractItemModel *tm = model();
#ifdef DEBUG
	QString name = tm->index (row, TreeModel::e_col_name, parentIndex).data ().toString ();
	QString parentName = tm->index (parentIndex.row (), TreeModel::e_col_name, parentIndex.parent ()).data ().toString ();
#endif

	bool isIndividual = tm->index (row, TreeModel::e_col_isIndividual, parentIndex).data ().toBool ();
	bool isDummy = tm->index (row, TreeModel::e_col_individualIsDummy, parentIndex).data ().toBool ();
	bool isSpouseItem = tm->index (row, TreeModel::e_col_isSpouseItem, parentIndex).data ().toBool ();
	QString uuid = getStringUuid (currentIndex);
	QString parentUuid = parentIndex.isValid () ? getStringUuid (parentIndex) : "RootItem";
	int childCountX = tm->index (row, TreeModel::e_col_childCount, parentIndex).data ().toInt ();
	bool hasBirthEvent = tm->index (row, TreeModel::e_col_hasBirthEvent, parentIndex).data ().toBool ();
	bool hasDeathEvent = tm->index (row, TreeModel::e_col_hasDeathEvent, parentIndex).data ().toBool ();
	bool hasMarriageEvent = tm->index (row, TreeModel::e_col_hasMarriageEvent, parentIndex).data ().toBool ();

	qreal cellWidthExcludingIcon = 0;
	qreal itemWidth = 0;
	qreal itemHeight = 0;
	qreal itemVerticalSeparation = 2 * _itemSeparation;

	QRectF parentRect = _rectForItem.value (parentUuid);
	qreal y_ = parentRect.y () + parentRect.height ();

	qreal y = y_ + itemVerticalSeparation;

	if (_showBasicDetailsForItem && _showExtraDetailsForItem && isIndividual && !isDummy)
	{
		_rectForIndex [tm->index (row, TreeModel::e_col_icon, parentIndex)]
				= QRectF (0, y + _lineSeparation, _iconSize, _iconSize);
		itemWidth += _lineSeparation + _iconSize;
	}

	if (!_showBasicDetailsForItem || (isIndividual && isDummy))
	{
		QModelIndex index = tm->index (row, TreeModel::e_col_name, parentIndex);
		_rectForIndex [index] = QRectF (0, y, _dummyDotSize, _dummyDotSize);
		y += _dummyDotSize;
		cellWidthExcludingIcon = _dummyDotSize;
	}
	else
	{
		QModelIndex index = tm->index (row, TreeModel::e_col_name, parentIndex);
		cellWidthExcludingIcon = qMax (cellWidthExcludingIcon, setRectForField (index, y));
	}

	if (_showBasicDetailsForItem && _showExtraDetailsForItem && isIndividual && !isDummy)
	{
		if (hasBirthEvent)
		{
			QModelIndex index = tm->index (row, TreeModel::e_col_ftBirthDate, parentIndex);
			cellWidthExcludingIcon = qMax (cellWidthExcludingIcon, setRectForField (index, y));
		}
		if (hasDeathEvent)
		{
			QModelIndex index = tm->index (row, TreeModel::e_col_ftDeathDate, parentIndex);
			cellWidthExcludingIcon = qMax (cellWidthExcludingIcon, setRectForField (index, y));
		}
		if (isSpouseItem && hasMarriageEvent)
		{
			QModelIndex index = tm->index (row, TreeModel::e_col_ftMarriageDate, parentIndex);
			cellWidthExcludingIcon = qMax (cellWidthExcludingIcon, setRectForField (index, y));
		}
	}

	y += _lineSeparation;

	itemHeight = y - y_ - _itemSeparation;
	qreal itemHeightWithIconOnly = _iconSize + 2 * _lineSeparation;

	if (_showBasicDetailsForItem && _showExtraDetailsForItem &&
	    isIndividual && !isDummy && itemHeightWithIconOnly > itemHeight)
		itemHeight = itemHeightWithIconOnly;
	itemWidth += _lineSeparation + cellWidthExcludingIcon + _lineSeparation;

	if (_showBasicDetailsForItem && (!isIndividual || !isDummy ))
		_rectForItem [uuid] = QRectF (0, y_ + itemVerticalSeparation, itemWidth, itemHeight);
	else
		_rectForItem [uuid] = QRectF (0, y_ + itemVerticalSeparation, _dummyDotSize, _dummyDotSize);
	_idealHeight = qMax (_idealHeight, y_ + itemHeight + 2 * itemVerticalSeparation);

	if (isExpanded (currentIndex))
	{
		for (int i = 0; i < childCountX; i++)
			calculateRectsIfRequired (tm->index (i, TreeModel::e_col_name, currentIndex));
	}

	qreal x_ = 0;
	qreal x = 0;
	if (childCountX && isExpanded (currentIndex))
	{
		QModelIndex leftMostDescendent = leftMostChild (currentIndex);
		while (tm->hasChildren (leftMostDescendent) && isExpanded (leftMostDescendent))
			leftMostDescendent = leftMostChild (leftMostDescendent);

		QModelIndex rightMostDescendent = rightMostChild (currentIndex);
		while (tm->hasChildren (rightMostDescendent) && isExpanded (rightMostDescendent))
			rightMostDescendent = rightMostChild (rightMostDescendent);

		QString leftMostDescendentUuid = getStringUuid (leftMostDescendent);
		QString rightMostDescendentUuid = getStringUuid (rightMostDescendent);
		qreal xMid = (_rectForItem.value (leftMostDescendentUuid).left () +
			      _rectForItem.value (rightMostDescendentUuid).right ()) / 2;
		x_ = xMid - (itemWidth / 2);
		x = x_;
	}
	else
	{
		if (isIndividual)
		{
			qreal parentMaxWidth = 0;

			QModelIndex tempChildIndex = currentIndex;
			QModelIndex tempIndex = parentIndex;
			while (tempIndex.isValid () && leftMostChild (tempIndex) == tempChildIndex)
			{
				QString tempUuid = getStringUuid (tempIndex);
				qreal parentWidth = _rectForItem.value (tempUuid).width ();
				if (parentMaxWidth < parentWidth)
					parentMaxWidth = parentWidth;
				if (tm->index (tempIndex.row (), TreeModel::e_col_isFamily, tempIndex.parent ()).data ().toBool ())
					break;
				tempChildIndex = tempIndex;
				tempIndex = tempIndex.parent ();
			}

			if (leftMostChild (tempIndex) == tempChildIndex)
				x_ = 0;
			else
			{
				QModelIndex childIndex = leftChildFrom (tempIndex, tempChildIndex);
				while (childIndex.isValid ())
				{
					QString childUuid = getStringUuid (childIndex);
					qreal xForChild = _rectForItem.value (childUuid).right ();
					if (x_ < xForChild)
						x_ = xForChild;
					if (!isExpanded (childIndex))
						break;

					childIndex = rightMostChild (childIndex);
				}
			}

			if (parentMaxWidth > itemWidth)
				x_ += + (parentMaxWidth - itemWidth) / 2 + _itemSeparation;
		}
		else
			x_ = 0;
		x = x_ + _itemSeparation;
	}

	QModelIndex tindex;
	qreal width;

	if (_showBasicDetailsForItem && _showExtraDetailsForItem && isIndividual && !isDummy)
	{
		x += _lineSeparation;
		tindex = tm->index (row, TreeModel::e_col_icon, parentIndex);
		width = _rectForIndex [tindex].width ();
		_rectForIndex [tindex].setLeft (x);
		_rectForIndex [tindex].setWidth (width);
		x += width;
	}

	x += _lineSeparation;

	tindex = tm->index (row, TreeModel::e_col_name, parentIndex);
	width = _rectForIndex [tindex].width ();
	_rectForIndex [tindex].setLeft (x);
	_rectForIndex [tindex].setWidth (width);

	if (_showBasicDetailsForItem && _showExtraDetailsForItem && isIndividual && !isDummy)
	{
		if (hasBirthEvent)
		{
			tindex = tm->index (row, TreeModel::e_col_ftBirthDate, parentIndex);
			width = _rectForIndex [tindex].width ();
			_rectForIndex [tindex].setLeft (x);
			_rectForIndex [tindex].setWidth (width);
		}

		if (hasDeathEvent)
		{
			tindex = tm->index (row, TreeModel::e_col_ftDeathDate, parentIndex);
			width = _rectForIndex [tindex].width ();
			_rectForIndex [tindex].setLeft (x);
			_rectForIndex [tindex].setWidth (width);
		}

		if (isSpouseItem && hasMarriageEvent)
		{
			tindex = tm->index (row, TreeModel::e_col_ftMarriageDate, parentIndex);
			width = _rectForIndex [tindex].width ();
			_rectForIndex [tindex].setLeft (x);
			_rectForIndex [tindex].setWidth (width);
		}
	}

	x += cellWidthExcludingIcon + _lineSeparation;

	if (childCountX && isExpanded (currentIndex))
		_rectForItem [uuid].setX (x_);
	else
		_rectForItem [uuid].setX (x_ + _itemSeparation);
	_rectForItem [uuid].setWidth (itemWidth);

	DEBUGTHIS ("Name: " << name << "Parent: " << parentName << "Children: " << childCountX);
	DEBUGVARIABLEANDITSVALUE (_rectForItem [uuid].x ());
	DEBUGVARIABLEANDITSVALUE (_rectForItem [uuid].y ());

	if (_idealWidth - _itemSeparation < x)
		_idealWidth = x + _itemSeparation;
}

QModelIndex FamilyTreeView::indexAtForFields (const QPoint &point, QModelIndex &index) const
{
	QModelIndex idx = index;
	QRectF rect = _rectForIndex.value (idx);
	if (rect.contains (point))
		return idx;

	QAbstractItemModel *tm = model ();
	int row = index.row ();
	QModelIndex parent = index.parent ();
	bool isIndividual = tm->index (row, TreeModel::e_col_isIndividual, parent).data ().toBool ();
	bool isDummy = tm->index (row, TreeModel::e_col_individualIsDummy, parent).data ().toBool ();

	if (_showExtraDetailsForItem && isIndividual && !isDummy)
	{
		idx = model ()->index (row, TreeModel::e_col_ftBirthDate, parent);
		rect = _rectForIndex.value (idx);
		if (rect.contains (point))
			return idx;

		idx = model ()->index (row, TreeModel::e_col_ftDeathDate, parent);
		rect = _rectForIndex.value (idx);
		if (rect.contains (point))
			return idx;

		idx = model ()->index (row, TreeModel::e_col_ftMarriageDate, parent);
		rect = _rectForIndex.value (idx);
		if (rect.contains (point))
			return idx;

		idx = model ()->index (row, TreeModel::e_col_icon, parent);
		rect = _rectForIndex.value (idx);
		if (rect.contains (point))
			return idx;
	}

	return QModelIndex ();
}

QModelIndex FamilyTreeView::indexAtRecursive (const QPoint &point, QModelIndex &index) const
{
	if (index.isValid ())
	{
		QModelIndex foundIdx = indexAtForFields (point, index);
		if (foundIdx.isValid ())
			return foundIdx;
	}

	for (int i = 0; i < model ()->rowCount (index); i++)
	{
		QModelIndex chIdx = model ()->index (i, TreeModel::e_col_name, index);
		QModelIndex foundIdx = indexAtRecursive (point, chIdx);
		if (foundIdx.isValid ())
			return foundIdx;
	}

	return QModelIndex();
}

bool FamilyTreeView::setSelectionRecursively (const QModelIndex &index, const QRect &rectangle,
					      QItemSelectionModel::SelectionFlags flags)
{
	bool ret = false;

	int firstRow = model()->rowCount(index);
	int lastRow = -1;
	for (int row = 0; row < model ()->rowCount (index); row++)
	{
		QModelIndex idx = model ()->index (row, TreeModel::e_col_name, index);
		if (setSelectionRecursively (idx, rectangle, flags))
			ret = true;

		QRectF rect = _rectForIndex.value (idx);
		if (!rect.intersects (rectangle))
			continue;

		idx = model ()->index (row, TreeModel::e_col_ftBirthDate, index);
		rect = _rectForIndex.value (idx);
		if (rect.intersects (rectangle))
			continue;

		idx = model ()->index (row, TreeModel::e_col_ftDeathDate, index);
		rect = _rectForIndex.value (idx);
		if (rect.contains (rectangle))
			continue;

		idx = model ()->index (row, TreeModel::e_col_ftMarriageDate, index);
		rect = _rectForIndex.value (idx);
		if (rect.contains (rectangle))
			continue;

		idx = model ()->index (row, TreeModel::e_col_icon, index);
		rect = _rectForIndex.value (idx);
		if (rect.contains (rectangle))
			continue;

		if (firstRow > row)
			firstRow = row;
		if (lastRow < row)
			lastRow = row;
	}

	if (firstRow != model()->rowCount() && lastRow != -1)
	{
		QItemSelection selection(model()->index(firstRow, 0, index),
					 model()->index(lastRow, 0, index));
		selectionModel()->select(selection, flags);
		ret = true;
	}

	return ret;
}

void FamilyTreeView::paintIndex (QModelIndex index, QPainter &painter)
{
	QRectF rect = viewportRectForIndex (index);
	if (rect.isValid () && viewport ()->rect ().intersects (rect.toRect ()))
	{
		QStyleOptionViewItem option = viewOptions();
		option.rect = rect.toRect ();
		if (selectionModel()->isSelected(index))
			option.state |= QStyle::State_Selected;
		if (currentIndex() == index)
			option.state |= QStyle::State_HasFocus;
		itemDelegate()->paint(&painter, option, index);
	}
}

void FamilyTreeView::paintIndexesRecursively (QModelIndex index, QPainter &painter)
{
	QAbstractItemModel *tm = model ();
	if (index.isValid ())
	{
		QModelIndex parentIndex = index.parent ();
		int row = index.row ();
		bool isIndividual = tm->index (row, TreeModel::e_col_isIndividual, parentIndex).data ().toBool ();
		bool isDummy = tm->index (row, TreeModel::e_col_individualIsDummy, parentIndex).data ().toBool ();
		bool isSpouseItem = tm->index (row, TreeModel::e_col_isSpouseItem, parentIndex).data ().toBool ();
		bool hasBirthEvent = tm->index (row, TreeModel::e_col_hasBirthEvent, parentIndex).data ().toBool ();
		bool hasDeathEvent = tm->index (row, TreeModel::e_col_hasDeathEvent, parentIndex).data ().toBool ();
		bool hasMarriageEvent = tm->index (row, TreeModel::e_col_hasMarriageEvent, parentIndex).data ().toBool ();
		QString uuid = getStringUuid (index);

		if (_showBasicDetailsForItem && (!isIndividual || !isDummy))
			paintIndex (tm->index (row, TreeModel::e_col_name, parentIndex), painter);
		else
		{
			QVariant foregroundColor = index.data (Qt::ForegroundRole);
			QColor color = foregroundColor.value<QColor>();
			QRectF rect = viewportRectForIndex (index);
			painter.save();
			QBrush brush;
			if (color.isValid ())
				brush = QBrush (color);
			else
				brush = QBrush (Qt::red);
			qreal brushSize;
			if (_showBasicDetailsForItem)
				brushSize = 8.0;
			else if (isDummy)
				brushSize = 1.0;
			else
				brushSize = 4.0;
			painter.setPen (QPen (brush, brushSize));
			painter.drawEllipse (rect.toRect ());
			painter.restore ();
		}

		if (_showBasicDetailsForItem && _showExtraDetailsForItem && isIndividual && !isDummy)
		{
			QModelIndex imageIndex = tm->index (row, TreeModel::e_col_icon, parentIndex);
			QRectF target = viewportRectForIndex (imageIndex);
			if (target.isValid () && viewport ()->rect ().intersects (target.toRect ()))
			{
				QRectF source (0, 0, _iconSize, _iconSize);
				QVariant value = tm->data (imageIndex, Qt::DecorationRole);
				if (value.canConvert <QIcon>())
				{
					QIcon icon = value.value <QIcon>();
					painter.drawPixmap (target, icon.pixmap (_iconSize, _iconSize), source);
				}
			}

			if (hasBirthEvent)
				paintIndex (tm->index (row, TreeModel::e_col_ftBirthDate, parentIndex), painter);
			if (hasDeathEvent)
				paintIndex (tm->index (row, TreeModel::e_col_ftDeathDate, parentIndex), painter);
			if (isSpouseItem && hasMarriageEvent)
				paintIndex (tm->index (row, TreeModel::e_col_ftMarriageDate, parentIndex), painter);
		}

		QRect itemRect = viewportRectForItem (index).toRect ();
		qreal xmid = (itemRect.left () + itemRect.right ()) / 2;
		qreal ytop = itemRect.top ();
		qreal ybot = itemRect.bottom ();
		qreal xvLeft = viewport ()->rect ().left ();
		qreal xvRight = viewport ()->rect ().right ();

		painter.save();
		painter.setPen(QPen(Qt::white, 0.5));

		if ((_showBasicDetailsForItem && (!isIndividual || !isDummy)) &&
		    itemRect.isValid () && viewport ()->rect ().intersects (itemRect))
			paintOutline (painter, itemRect);
		else if (_showBasicDetailsForItem && isIndividual)
		{
			QModelIndex leftSibling = leftChildFrom (parentIndex, index);
			QModelIndex rightSibling = rightChildFrom (parentIndex, index);

			if (leftSibling.isValid ())
			{
				QRect rectForSibling = viewportRectForItem (leftSibling).toRect ();
				if (viewport ()->rect ().contains (rectForSibling) ||
						(rectForSibling.left () < xvLeft && itemRect.right () > xvRight))
				{
					QString name = index.data ().toString ();
					painter.drawText (xvRight - 200 - _itemSeparation / 2,
							  ytop - 2 * _itemSeparation,
							  200, 20, Qt::AlignRight | Qt::AlignTop,
							  name + " ->");
				}
			}

			if (rightSibling.isValid ())
			{
				QRect rectForSibling = viewportRectForItem (rightSibling).toRect ();
				if (viewport ()->rect ().contains (rectForSibling) ||
						(itemRect.left () < xvLeft && rectForSibling.right () > xvRight))
				{
					QString name = index.data ().toString ();
					painter.drawText (xvLeft + _itemSeparation / 2,
							  ytop - _itemSeparation / 2 - 2,
							  "<- " + name);
				}
			}
		}
		if (itemRect.isValid () && !viewport ()->rect ().contains (itemRect))
		{
			if (_showBasicDetailsForItem && !isIndividual)
			{
				if (xvLeft < itemRect.left ())
				{
					QString name = index.data ().toString ();
					painter.drawText (xvLeft + _itemSeparation / 2,
							  ytop - _itemSeparation / 2 -2,
							  name + " ->");
				}
				else
				{
					QString name = index.data ().toString ();
					painter.drawText (xvRight - 200 - _itemSeparation / 2,
							  ytop - 2 * _itemSeparation,
							  200, 20, Qt::AlignRight | Qt::AlignTop,
							  "<- " + name);
				}
			}
		}

		painter.drawLine (xmid, ytop, xmid, ytop - _itemSeparation);

		if (tm->hasChildren (index))
		{
			if (isExpanded (index))
			{
				painter.drawLine (xmid, ybot, xmid, ybot + _itemSeparation + 1);
				if (ybot + _itemSeparation < viewport ()->rect ().bottom () &&
				    ybot + _itemSeparation > viewport ()->rect ().top () &&
				    tm->rowCount (index) > 1)
				{
					QRectF leftMostChildRect = viewportRectForItem (leftMostChild (index));
					QRectF rightMostChildRect = viewportRectForItem (rightMostChild (index));
					qreal xLeft = (leftMostChildRect.left () + leftMostChildRect.right ()) / 2;
					qreal xRight = (rightMostChildRect.left () + rightMostChildRect.right ()) / 2;
					qreal x1 = 0;
					qreal x2 = 0;
					if (xLeft < xvLeft)
						x1 = xvLeft;
					else
						x1 = xLeft;
					if (xRight > xvRight)
						x2 = xvRight;
					else
						x2 = xRight;
					painter.drawLine (x1, ybot + _itemSeparation, x2, ybot + _itemSeparation);
				}
			}
			else
			{
				QRect rectangle = viewportRectForItemTerminator (uuid).toRect ();
				paintTreeBranchTerminationForExpandable (painter, rectangle);
			}
		}
		else if (_showBasicDetailsForItem)
		{
			QRect rectangle = viewportRectForItemTerminator (uuid).toRect ();
			bool paintForDaughter = false;
			if (isIndividual && !isSpouseItem &&
					tm->index (row, TreeModel::e_col_female, parentIndex).data ().toBool ())
				paintForDaughter = true;

			if (paintForDaughter)
				paintTreeBranchTerminationForDaughters (painter, rectangle);
			else
				paintTreeBranchTerminationForAll (painter, rectangle);
		}

		if (!isIndividual)
		{
			QRectF parentRect = viewportRectForItem (parentIndex);
			qreal xparentMid = (parentRect.left () + parentRect.right ()) / 2;
			qreal x1 = 0;
			qreal x2 = 0;
			if (xmid < xvLeft)
				x1 = xvLeft;
			else
				x1 = xmid;
			if (xparentMid > xvRight)
				x2 = xvRight;
			else
				x2 = xparentMid;
			painter.drawLine (x1, ytop - _itemSeparation - 1, x2, ytop - _itemSeparation - 1);
		}

		painter.restore();
	}

	if (isExpanded (index))
	{
		for (int i = 0; i < tm->rowCount (index); i++)
			paintIndexesRecursively (tm->index (i, TreeModel::e_col_name, index), painter);
	}
}

void FamilyTreeView::paintOutline(QPainter &painter, const QRect &rectangle)
{
	const QRect rect = rectangle.adjusted(0, 0, -1, -1);
	painter.save();
	painter.setPen(QPen(palette().dark().color(), 0.5));
	painter.drawRect(rect);
	painter.setPen(QPen(Qt::black, 0.5));
	painter.drawLine(rect.bottomLeft(), rect.bottomRight());
	painter.drawLine(rect.bottomRight(), rect.topRight());
	painter.restore();
}

void FamilyTreeView::paintTreeBranchTerminationForDaughters (QPainter &painter, const QRect &rectangle)
{
	qreal top = rectangle.top ();
	qreal xmid = (rectangle.left () + rectangle.right ()) / 2;
	int radius = _itemSeparation - _yStep;
	QPoint center (xmid, top + _yStep + radius);
	QPoint crossLineP1 (center.x () + radius * qCos (M_PI / 4),
			    center.y () - radius * qSin (M_PI / 4));
	QPoint crossLineP2 (center.x () - radius * qCos (M_PI / 4),
			    center.y () + radius * qSin (M_PI / 4));

	painter.save ();
	painter.setPen (QPen(Qt::red, 1.0));
	painter.drawEllipse (center, radius, radius);
	painter.drawLine (crossLineP1, crossLineP2);
	painter.restore ();
	painter.drawLine (xmid, top, xmid, top + _itemSeparation - _yStep - 1);
}

void FamilyTreeView::paintTreeBranchTerminationForAll (QPainter &painter, const QRect &rectangle)
{
	qreal top = rectangle.top ();
	qreal xmid = (rectangle.left () + rectangle.right ()) / 2;
	int xDiff = _itemSeparation - _xStep;
	int yDiff = _yStep;
	painter.save ();
	QPen pen;
	pen.setWidth (1.0);
	pen.setColor (QColor ("orange"));
	painter.setPen (pen);
	while (xDiff > 0)
	{
		painter.drawLine (xmid - xDiff, top + yDiff, xmid + xDiff, top + yDiff);
		xDiff -= _xStep;
		yDiff += _yStep;
	}
	painter.restore ();
	painter.drawLine (xmid, top, xmid, top + _itemSeparation - _yStep - 1);
}

void FamilyTreeView::paintTreeBranchTerminationForExpandable (QPainter &painter, const QRect &rectangle)
{
	qreal top = rectangle.top ();
	qreal xmid = (rectangle.left () + rectangle.right ()) / 2;
	qreal ymid = (rectangle.top () + rectangle.bottom ()) / 2 + 1;
	qreal plusLeft = xmid - _plusLength;
	qreal plusRight = xmid + _plusLength;
	qreal plusTop = top + _yStep + _plusPadding;
	qreal plusBot = plusTop + 2 * _plusLength;
	qreal rectLeft = plusLeft - _plusPadding;
	qreal rectTop = top + _yStep;
	qreal rectWidth = 2 * (_plusLength + _plusPadding);
	qreal rectHeight = rectWidth;

	painter.save ();
	painter.setPen (QPen(Qt::green, 1.0));
	painter.drawLine (plusLeft, ymid, plusRight, ymid);
	painter.drawLine (xmid, plusTop, xmid, plusBot);
	painter.restore ();

	painter.drawLine (xmid, top, xmid, rectTop - 1);
	painter.drawRect (rectLeft, rectTop, rectWidth, rectHeight);
}

void FamilyTreeView::expandAll (const QModelIndex &index)
{
	if (!index.isValid ())
		return;

	setExpanded (index, true, false);
	QAbstractItemModel *tm = model ();
	for (int i = 0; i < tm->rowCount (index); i++)
		expandAll (tm->index (i, index.column (), index));
}

QModelIndex FamilyTreeView::leftMostChild (const QModelIndex &parent) const
{
	if (!model ()->hasChildren (parent))
		return QModelIndex ();

	QAbstractItemModel *tm = model ();
	return tm->index (0, parent.column (), parent);
}

QModelIndex FamilyTreeView::rightMostChild (const QModelIndex &parent) const
{
	if (!model ()->hasChildren (parent))
		return QModelIndex ();

	QAbstractItemModel *tm = model ();
	return tm->index (tm->rowCount (parent) - 1, parent.column (), parent);
}

QModelIndex FamilyTreeView::leftChildFrom (const QModelIndex &parent, const QModelIndex &child) const
{
	int chNo = childNumber (parent, child);

	if (chNo <= 0)
		return QModelIndex ();

	return model ()->index (chNo - 1, parent.column (), parent);
}

QModelIndex FamilyTreeView::rightChildFrom (const QModelIndex &parent, const QModelIndex &child) const
{
	int chNo = childNumber (parent, child);

	if (chNo < 0 || chNo == model ()->rowCount (parent) - 1)
		return QModelIndex ();

	return model ()->index (chNo + 1, parent.column (), parent);
}

int FamilyTreeView::childNumber (const QModelIndex &parent, const QModelIndex &child) const
{
	if (child.parent () != parent)
		return -1;

	QAbstractItemModel *tm = model ();
	QModelIndex tChild = tm->index (child.row (), TreeModel::e_col_name, child.parent ());
	for (int i = 0; i < tm->rowCount (parent); i++)
		if (tChild == tm->index (i, TreeModel::e_col_name, parent))
			return i;
	return -1;
}

QString FamilyTreeView::getStringUuid (const QModelIndex &index) const
{
	QAbstractItemModel *tm = model ();
	QModelIndex uuidIndex = tm->index (index.row (), TreeModel::e_col_uniqueIdentifier, index.parent ());
	return uuidIndex.data ().toString ();
}

bool FamilyTreeView::isExpanded (const QModelIndex &index) const
{
	if (!index.isValid ())
		return true;

	return model ()->index (index.row (), TreeModel::e_col_expandable, index.parent ()).data ().toBool ();
}

void FamilyTreeView::setExpanded (const QModelIndex &index, const bool expanded,
				  const bool updateViewPort) const
{
	if (!index.isValid ())
		return;

	QAbstractItemModel *tm = model ();
	QModelIndex expandableIndex = tm->index (index.row (), TreeModel::e_col_expandable, index.parent ());
	bool stateChanged = tm->setData (expandableIndex, expanded);
	if (!stateChanged)
		return;

	_hashNeedsUpdate = true;
	if (updateViewPort)
		viewport ()->update ();
}

void FamilyTreeView::edit (const QModelIndex &index)
{
	QAbstractItemView::edit (index);
}

void FamilyTreeView::reset ()
{
	_hashNeedsUpdate = true;
	QAbstractItemView::reset ();
	viewport ()->update ();
}

void FamilyTreeView::collapse (const QModelIndex &index)
{
	setExpanded (index, false);
}

void FamilyTreeView::collapseAll ()
{
	collapse (rootIndex ());
}

void FamilyTreeView::expand (const QModelIndex &index)
{
	setExpanded (index, true);
}

void FamilyTreeView::expandAll ()
{
	expandAll (rootIndex ());
	_hashNeedsUpdate = true;
	viewport ()->update ();
}

void FamilyTreeView::setShowExtraDetailsForItem (const bool showExtraDetailsForItem)
{
	_showExtraDetailsForItem = showExtraDetailsForItem;
	_hashNeedsUpdate = true;
	viewport ()->update ();
}

void FamilyTreeView::setShowBasicDetailsForItem (const bool showBasicDetailsForItem)
{
	_showBasicDetailsForItem = showBasicDetailsForItem;
	_hashNeedsUpdate = true;
	viewport ()->update ();
}
