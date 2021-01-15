﻿#include "SARibbonPannel.h"
#include "SARibbonToolButton.h"
#include <QApplication>
#include <QResizeEvent>
#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QGridLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include "SARibbonPannelOptionButton.h"
#include "SARibbonSeparatorWidget.h"
#include "SARibbonGallery.h"
#include "SARibbonElementManager.h"
#include "SARibbonMenu.h"
#include <QWidgetAction>

#define HELP_DRAW_RECT(p, rect)		 \
    do{				 \
        p.save();		 \
        QPen _pen(Qt::DashLine); \
        _pen.setColor(Qt::blue); \
        p.setPen(_pen);		 \
        p.setBrush(QBrush());	 \
        p.drawRect(rect);	 \
        p.restore();		 \
    }while(0)

const int c_higherModehight = 98;
const int c_lowerModehight = 72;
const int c_iconHighForHigerLarge = 32;
const QSize c_iconSizeForHigerLarge = QSize(c_iconHighForHigerLarge, c_iconHighForHigerLarge);
const int c_iconHighForHigerSmall = 16;
const QSize c_iconSizeForHigerSmall = QSize(c_iconHighForHigerSmall, c_iconHighForHigerSmall);




class SARibbonPannelPrivate
{
public:
    SARibbonPannelPrivate(SARibbonPannel *p);
    //根据m_pannelLayoutMode返回gridLayout应该增加的行数
    int rowadded();
    void createLayout();
    void recalcTitleY();

    SARibbonPannel *Parent;
    QGridLayout *m_gridLayout;
    QPoint m_nextElementPosition;
    int m_row;                                              ///< 记录小action所在的gridLayout行数，gridLayout总共划分为6行，用于满足3行或2行的按钮需求
    SARibbonPannelOptionButton *m_optionActionButton;
    int m_titleY;                                           ///< 标题栏的y距离
    SARibbonPannel::PannelLayoutMode m_pannelLayoutMode;    ///< pannel的布局模式，默认为3行模式ThreeRowMode
};

SARibbonPannelPrivate::SARibbonPannelPrivate(SARibbonPannel *p)
    : Parent(p)
    , m_gridLayout(nullptr)
    , m_nextElementPosition(3, 3)
    , m_row(0)
    , m_optionActionButton(nullptr)
    , m_titleY(77)
    , m_pannelLayoutMode(SARibbonPannel::ThreeRowMode)
{
    createLayout();
    recalcTitleY();
}


int SARibbonPannelPrivate::rowadded()
{
    switch (m_pannelLayoutMode)
    {
    case SARibbonPannel::ThreeRowMode:
        return (2);

    case SARibbonPannel::TwoRowMode:
        return (3);

    default:
        break;
    }
    return (2);
}


void SARibbonPannelPrivate::createLayout()
{
    if (m_gridLayout) {
        delete m_gridLayout;
    }
    m_gridLayout = new QGridLayout(Parent);
    m_gridLayout->setSpacing(2);
    if (SARibbonPannel::TwoRowMode == m_pannelLayoutMode) {
        m_gridLayout->setContentsMargins(RibbonSubElementStyleOpt.pannelLayoutMargins2Rows);
    }else{
        m_gridLayout->setContentsMargins(RibbonSubElementStyleOpt.pannelLayoutMargins3Rows);
    }
}


void SARibbonPannelPrivate::recalcTitleY()
{
    if (SARibbonPannel::ThreeRowMode == m_pannelLayoutMode) {
        m_titleY = Parent->height()-RibbonSubElementStyleOpt.pannelTitleHeight;
    }else {
        m_titleY = Parent->height()*0.94;
    }
}




SARibbonPannelItem::SARibbonPannelItem(QWidget *widget):QWidgetItem(widget)
    ,action(nullptr)
    ,customWidget(false)
{

}

bool SARibbonPannelItem::isEmpty() const
{
    return action == 0 || !action->isVisible();
}




SARibbonPannelLayout::SARibbonPannelLayout(QWidget *p):QLayout(p)
  ,m_expandFlag(false)
{
    SARibbonPannel *tb = qobject_cast<SARibbonPannel*>(p);
    if (!tb){
        return;
    }

}

SARibbonPannelLayout::~SARibbonPannelLayout()
{

}

void SARibbonPannelLayout::addItem(QLayoutItem *item)
{
    qWarning(tr("SARibbonPannelLayout::addItem(): please use addAction() instead"));
    return;
}

QLayoutItem *SARibbonPannelLayout::itemAt(int index) const
{
    if (index < 0 || index >= m_items.count())
        return nullptr;
    return m_items.at(index);
}

QLayoutItem *SARibbonPannelLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.count()){
        return nullptr;
    }
    SARibbonPannelItem *item = m_items.takeAt(index);

    QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(item->action);
    if (widgetAction != 0 && item->customWidget) {
        widgetAction->releaseWidget(item->widget());
    } else {
        // destroy the QToolButton/QToolBarSeparator
        item->widget()->hide();
        item->widget()->deleteLater();
    }

    invalidate();
    return item;
}

int SARibbonPannelLayout::count() const
{
    return m_items.count();
}

bool SARibbonPannelLayout::isEmpty() const
{
    return m_items.isEmpty();
}

void SARibbonPannelLayout::invalidate()
{
    QLayout::invalidate();
}

Qt::Orientations SARibbonPannelLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

QSize SARibbonPannelLayout::minimumSize() const
{

}

QSize SARibbonPannelLayout::sizeHint() const
{

}

bool SARibbonPannelLayout::isExpand() const
{

}

/**
 * @brief 把action转换为item，此函数会创建SARibbonToolButton
 *
 * 此函数参考QToolBarItem *QToolBarLayout::createItem(QAction *action)
 * @param action
 * @return
 */
SARibbonPannelItem *SARibbonPannelLayout::createItem(QAction *action)
{
    bool customWidget = false;
    QWidget *widget = nullptr;
    SARibbonPannel *pannel = qobject_cast<SARibbonPannel*>(parentWidget());
    if (!pannel){
            return nullptr;
    }
    if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action)) {
        widget = widgetAction->requestWidget(pannel);
        if (widget != nullptr) {
            widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
            customWidget = true;
        }
    } else if (action->isSeparator()) {
        SARibbonSeparatorWidget *sep = new SARibbonSeparatorWidget(pannel);
        widget = sep;
    }
    //不是widget，自动生成SARibbonToolbutton
    if (!widget) {
        SARibbonToolButton *button = RibbonSubElementDelegate->createRibbonToolButton(pannel);
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);

        button->setDefaultAction(action);
        QObject::connect(button, &SARibbonToolButton::triggered
                         , pannel,&SARibbonPannel::actionTriggered);
        widget = button;
    }
    //这时总会有widget
    widget->hide();
    SARibbonPannelItem *result = new SARibbonPannelItem(widget);
    result->customWidget = customWidget;
    result->action = action;
    return result;
}

/**
 * @brief 更新尺寸
 */
void SARibbonPannelLayout::updateGeomArray() const
{
    SARibbonPannel *pannel = qobject_cast<SARibbonPannel*>(parentWidget());
    if (!pannel)
        return;
    QMargins mag = this->contentsMargins();
    const int margin = this->margin();
    const int spacing = this->spacing();
    for (int i = 0; i < m_items.count(); ++i) {
        SARibbonPannelItem *item = m_items.at(i);
        QSize max = item->maximumSize();
        QSize min = item->minimumSize();
        QSize hint = item->sizeHint();
        Qt::Orientations exp = item->expandingDirections();
        if (item->widget()) {
            //有窗口是水平扩展，则标记为扩展
            if ((item->widget()->sizePolicy().horizontalPolicy() & QSizePolicy::ExpandFlag)) {
                m_expandFlag = true;
            }
        }
    }
}


void SARibbonPannelLayout::setGeometry(const QRect &rect)
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    const int margin = this->margin();
    const int spacing = this->spacing();
    for (int i = 0; i < m_items.count(); ++i) {
        SARibbonPannelItem *item = m_items.at(i);
    }
}










SARibbonPannel::SARibbonPannel(QWidget *parent) : QWidget(parent)
    , m_d(new SARibbonPannelPrivate(this))
{
    setPannelLayoutMode(ThreeRowMode);
}


SARibbonPannel::~SARibbonPannel()
{
    delete m_d;
}


SARibbonToolButton *SARibbonPannel::addLargeAction(QAction *action)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::LargeButton);
    btn->setLargeButtonType(((ThreeRowMode == m_d->m_pannelLayoutMode) ? SARibbonToolButton::Normal : SARibbonToolButton::Lite));
    btn->setAutoRaise(true);
    btn->setDefaultAction(action);
    QSize iconSize = maxHightIconSize(action->icon().actualSize(c_iconSizeForHigerLarge), c_iconHighForHigerLarge);

    btn->setIconSize(iconSize);
    if (action->menu()) {
        btn->setPopupMode(QToolButton::MenuButtonPopup);
    }
    m_d->m_gridLayout->addWidget(btn, 0, m_d->m_gridLayout->columnCount(), 6, 1);
    m_d->m_row = 0;
    addAction(action);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addLargeToolButton(const QString& text, const QIcon& icon, QToolButton::ToolButtonPopupMode popMode)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::LargeButton);
    btn->setLargeButtonType(((ThreeRowMode == m_d->m_pannelLayoutMode) ? SARibbonToolButton::Normal : SARibbonToolButton::Lite));
    btn->setAutoRaise(true);
    QSize iconSize = maxHightIconSize(icon.actualSize(c_iconSizeForHigerLarge), c_iconHighForHigerLarge);

    btn->setIconSize(iconSize);
    btn->setIcon(icon);
    btn->setPopupMode(popMode);
    btn->setText(text);
    m_d->m_gridLayout->addWidget(btn, 0, m_d->m_gridLayout->columnCount(), 6, 1);
    m_d->m_row = 0;
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(btn);
    addAction(action);
    action->requestWidget(this);
    btn->setVisible(true);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addSmallToolButton(const QString& text, const QIcon& icon, QToolButton::ToolButtonPopupMode popMode)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::SmallButton);
    btn->setAutoRaise(true);
    QSize iconSize = maxHightIconSize(icon.actualSize(c_iconSizeForHigerSmall), c_iconHighForHigerSmall);

    btn->setIconSize(iconSize);
    btn->setIcon(icon);
    btn->setPopupMode(popMode);
    btn->setText(text);
    //
    if (0 == m_d->m_row) {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount(), 2, 1);
    }else{
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount()-1, 2, 1);
    }
    m_d->m_row += (m_d->rowadded());
    if (m_d->m_row >= 5) {//
        m_d->m_row = 0;
    }
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(btn);
    addAction(action);
    action->requestWidget(this);
    btn->setVisible(true);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addSmallAction(QAction *action)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::SmallButton);
    btn->setAutoRaise(true);
    btn->setDefaultAction(action);
    QSize iconSize = maxHightIconSize(action->icon().actualSize(c_iconSizeForHigerSmall), c_iconHighForHigerSmall);

    btn->setIconSize(iconSize);
    if (action->menu()) {
        btn->setPopupMode(QToolButton::MenuButtonPopup);
    }
    if (0 == m_d->m_row) {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount(), 2, 1);
    }else{
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount()-1, 2, 1);
    }
    m_d->m_row += (m_d->rowadded());
    if (m_d->m_row >= 5) {
        m_d->m_row = 0;
    }
    addAction(action);
    return (btn);
}


/**
 * @brief 在三栏模式下，强制加为2栏action
 *
 * 主要应用在ThreeRowMode下
 * @param action
 * @return
 */
SARibbonToolButton *SARibbonPannel::addMediumAction(QAction *action)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::SmallButton);
    btn->setAutoRaise(true);
    btn->setDefaultAction(action);
    QSize iconSize = maxHightIconSize(action->icon().actualSize(c_iconSizeForHigerSmall), c_iconHighForHigerSmall);

    btn->setIconSize(iconSize);
    if (action->menu()) {
        btn->setPopupMode(QToolButton::MenuButtonPopup);
    }
    if (0 == m_d->m_row) {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount(), 3, 1);
        m_d->m_row = 3;
    }else if (3 == m_d->m_row) {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount()-1, 3, 1);
        m_d->m_row += 3;
    }else {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount(), 3, 1);
        m_d->m_row = 0;
    }

    addAction(action);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addLargeMenu(SARibbonMenu *menu)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::LargeButton);
    btn->setLargeButtonType(((ThreeRowMode == m_d->m_pannelLayoutMode) ? SARibbonToolButton::Normal : SARibbonToolButton::Lite));
    btn->setAutoRaise(true);
    if (!menu->icon().isNull()) {
        QSize iconSize = maxHightIconSize(menu->icon().actualSize(c_iconSizeForHigerLarge), c_iconHighForHigerLarge);
        btn->setIconSize(iconSize);
        btn->setIcon(menu->icon());
    }
    btn->setPopupMode(QToolButton::InstantPopup);
    btn->setText(menu->title());
    btn->setMenu(menu);
    m_d->m_gridLayout->addWidget(btn, 0, m_d->m_gridLayout->columnCount(), 6, 1);
    m_d->m_row = 0;
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(btn);
    addAction(action);
    action->requestWidget(this);
    btn->setVisible(true);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addSmallMenu(SARibbonMenu *menu)
{
    SARibbonToolButton *btn = RibbonSubElementDelegate->createRibbonToolButton(this);

    btn->setButtonType(SARibbonToolButton::SmallButton);
    QSize iconSize = maxHightIconSize(menu->icon().actualSize(c_iconSizeForHigerSmall), c_iconHighForHigerSmall);

    btn->setIconSize(iconSize);
    btn->setMenu(menu);
    btn->setPopupMode(QToolButton::InstantPopup);
    if (0 == m_d->m_row) {
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount(), 2, 1);
    }else{
        m_d->m_gridLayout->addWidget(btn, m_d->m_row, m_d->m_gridLayout->columnCount()-1, 2, 1);
    }
    m_d->m_row += (m_d->rowadded());
    if (m_d->m_row >= 5) {
        m_d->m_row = 0;
    }
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(btn);
    addAction(action);
    action->requestWidget(this);
    btn->setVisible(true);
    return (btn);
}


SARibbonToolButton *SARibbonPannel::addLargeActionMenu(QAction *action, SARibbonMenu *menu)
{
    SARibbonToolButton *btn = addLargeAction(action);

    btn->setMenu(menu);
    btn->setPopupMode(QToolButton::MenuButtonPopup);
    return (btn);
}


SARibbonGallery *SARibbonPannel::addGallery()
{
    SARibbonGallery *gallery = RibbonSubElementDelegate->createRibbonGallery(this);

    m_d->m_gridLayout->addWidget(gallery, 0, m_d->m_gridLayout->columnCount(), 6, 1);
    m_d->m_row = 0;
    setExpanding();
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(gallery);
    addAction(action);
    action->requestWidget(this);
    gallery->setVisible(true);
    return (gallery);
}


/**
 * @brief 获取pannel下面的所有toolbutton
 * @return
 */
QList<SARibbonToolButton *> SARibbonPannel::toolButtons() const
{
    const QObjectList& objs = children();
    QList<SARibbonToolButton *> res;

    for (QObject *o : objs)
    {
        SARibbonToolButton *b = qobject_cast<SARibbonToolButton *>(o);
        if (b) {
            res.append(b);
        }
    }
    return (res);
}


/**
 * @brief SARibbonPannel::setPannelLayoutMode
 * @param mode
 */
void SARibbonPannel::setPannelLayoutMode(SARibbonPannel::PannelLayoutMode mode)
{
    if (m_d->m_pannelLayoutMode == mode) {
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
        qDebug() << QStringLiteral("setPannelLayoutMode 多次调用同一函数，跳过");
#endif
        return;
    }
    m_d->m_pannelLayoutMode = mode;
    int high = c_higherModehight;

    switch (mode)
    {
    case ThreeRowMode:
        high = c_higherModehight;
        break;

    case TwoRowMode:
        high = c_lowerModehight;
        break;

    default:
        high = c_higherModehight;
        break;
    }
    setFixedHeight(high);
    m_d->recalcTitleY();
    setMinimumWidth(50);
    resetLayout(mode);
    resetLargeToolButtonStyle();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //QApplication::postEvent(this, new QResizeEvent(newSize, oldSize));
}


SARibbonPannel::PannelLayoutMode SARibbonPannel::pannelLayoutMode() const
{
    return (m_d->m_pannelLayoutMode);
}

/**
 * @brief 添加分割线
 * @param top 上边距 @default 6
 * @param bottom 下边距 @default 6
 */
void SARibbonPannel::addSeparator(int top,int bottom)
{
    SARibbonSeparatorWidget *sep = RibbonSubElementDelegate->createRibbonSeparatorWidget(this);
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
    qDebug() << "SARibbonPannel height:" << height() << " sizeHint:" << sizeHint() << " geometry" << geometry();
    qDebug() << "SARibbonPannel addSeparator:" << separatorHeight();
#endif
    sep->setTopBottomMargins(6,6);
    m_d->m_gridLayout->addWidget(sep, 0, m_d->m_gridLayout->columnCount(), 6, 1);
    m_d->m_row = 0;
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(sep);
    addAction(action);
    action->requestWidget(this);
    sep->setVisible(true);
}


///
/// \brief 向pannel添加窗口，此窗口的所有权归SARibbonPannel所有
/// \param w
///
void SARibbonPannel::addSmallWidget(QWidget *w)
{
    if (w->parentWidget() != this) {
        w->setParent(this);
    }
    int col = m_d->m_gridLayout->columnCount();

    if (0 != m_d->m_row) {
        //如果没占满就在当前列里进行操作，col-1就是当前列，如果占满就新一列
        col -= 1;
    }
    m_d->m_gridLayout->addWidget(w, m_d->m_row, col, 2, 1);
    m_d->m_row += (m_d->rowadded());
    if (m_d->m_row >= 5) {
        m_d->m_row = 0;
    }
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(w);
    addAction(action);
    action->requestWidget(this);
    w->setVisible(true);
}


void SARibbonPannel::addLargeWidget(QWidget *w)
{
    if (w->parentWidget() != this) {
        w->setParent(this);
    }
    int col = m_d->m_gridLayout->columnCount();

    m_d->m_gridLayout->addWidget(w, 0, col, 6, 1);
    m_d->m_row = 0;
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(w);
    addAction(action);
    action->requestWidget(this);
    w->setVisible(true);
}


/**
 * @brief SARibbonPannel::addWidget
 * @param w
 * @param row gridlayout的行数，pannel的gridllayout一共有6行用于分割
 * @param rowSpan 窗口所占的行数，类似QGridLayout的rowSpan
 */
void SARibbonPannel::addWidget(QWidget *w, int row, int rowSpan)
{
    if (this != w->parentWidget()) {
        w->setParent(this);
    }
    int col = m_d->m_gridLayout->columnCount();

    if ((0 != row) && (0 != col)) {
        col -= 1;
    }
    m_d->m_gridLayout->addWidget(w, row, col, rowSpan, 1);
    m_d->m_row = row + rowSpan;
    if (m_d->m_row >= 5) {
        m_d->m_row = 0;
    }
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(w);
    addAction(action);
    action->requestWidget(this);
    w->setVisible(true);
}


/**
 * @brief SARibbonPannel::addWidget
 * @param w
 * @param row gridlayout的行数，pannel的gridllayout一共有6行用于分割
 * @param rowSpan 窗口所占的行数，类似QGridLayout的rowSpan
 * @param column 列位置，使用前先用 @ref gridLayoutColumnCount 返回当前的列数
 * @param columnSpan 窗口所占的列数，类似QGridLayout的columnSpan
 */
void SARibbonPannel::addWidget(QWidget *w, int row, int rowSpan, int column, int columnSpan)
{
    w->setParent(this);
    m_d->m_gridLayout->addWidget(w, row, column, rowSpan, columnSpan);
    m_d->m_row = row + rowSpan;
    if (row >= 5) {
        m_d->m_row = 0;
    }
    QWidgetAction* action = new QWidgetAction(this);
    action->setDefaultWidget(w);
    addAction(action);
    action->requestWidget(this);
    w->setVisible(true);
}


int SARibbonPannel::gridLayoutColumnCount() const
{
    return (m_d->m_gridLayout->columnCount());
}

/**
 * @brief 添加操作action，如果要去除，传入nullptr指针即可，SARibbonPannel不会对QAction的所有权进行管理
 * @param action
 * @note 要去除OptionAction直接传入nullptr即可
 * @note SARibbonPannel不对QAction的destroy进行关联，如果外部对action进行delete，需要先传入nullptr给addOptionAction
 */
void SARibbonPannel::addOptionAction(QAction *action)
{
    if (nullptr == action) {
        if (m_d->m_optionActionButton) {
            delete m_d->m_optionActionButton;
            m_d->m_optionActionButton = nullptr;
        }
        return;
    }
    if (nullptr == m_d->m_optionActionButton) {
        m_d->m_optionActionButton = RibbonSubElementDelegate->createRibbonPannelOptionButton(this);
        if (SARibbonPannel::TwoRowMode == pannelLayoutMode()) {
            QMargins m = RibbonSubElementStyleOpt.pannelLayoutMargins2Rows;
            m.setRight(m_d->m_optionActionButton->width());
            m_d->m_gridLayout->setContentsMargins(m);
        }
    }
    m_d->m_optionActionButton->connectAction(action);
    repaint();
}


QSize SARibbonPannel::maxHightIconSize(const QSize& size, int h)
{
    if (size.height() < h) {
        return (size * ((float)h/size.height()));
    }
    return (size);
}


void SARibbonPannel::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    //! 1. 绘制标题
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
    HELP_DRAW_RECT(p, rect());
#endif
    if (ThreeRowMode == pannelLayoutMode()) {
        QFont f = font();
        f.setPixelSize(RibbonSubElementStyleOpt.pannelTitleHeight * 0.6);
        p.setFont(f);
        if (m_d->m_optionActionButton) {
            p.drawText(1, height()-RibbonSubElementStyleOpt.pannelTitleHeight
                , width()- m_d->m_optionActionButton->width() - 4
                , RibbonSubElementStyleOpt.pannelTitleHeight
                , Qt::AlignCenter
                , windowTitle());
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
            QRect r = QRect(1, height()-RibbonSubElementStyleOpt.pannelTitleHeight
                , width()- m_d->m_optionActionButton->width() - 4
                , RibbonSubElementStyleOpt.pannelTitleHeight-2);
            HELP_DRAW_RECT(p, r);
#endif
        }else {
            p.drawText(1, height()-RibbonSubElementStyleOpt.pannelTitleHeight
                , width()
                , RibbonSubElementStyleOpt.pannelTitleHeight
                , Qt::AlignCenter
                , windowTitle());
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
            QRect r = QRect(1, height()-RibbonSubElementStyleOpt.pannelTitleHeight
                , width()
                , RibbonSubElementStyleOpt.pannelTitleHeight);
            HELP_DRAW_RECT(p, r);
#endif
        }
    }

    QWidget::paintEvent(event);
}


QSize SARibbonPannel::sizeHint() const
{
    QSize laySize = layout()->sizeHint();
    int maxWidth = laySize.width() + 2;
    if(ThreeRowMode == pannelLayoutMode()){
        //三行模式
        QFontMetrics fm = fontMetrics();
        QSize titleSize = fm.size(Qt::TextShowMnemonic, windowTitle());
        if(m_d->m_optionActionButton)
        {
            //optionActionButton的宽度需要预留
            titleSize.setWidth(titleSize.width() + m_d->m_optionActionButton->width() + 4);
        }
        maxWidth = qMax(maxWidth, titleSize.width());
    }
    return (QSize(maxWidth, laySize.height()));
}


QSize SARibbonPannel::minimumSizeHint() const
{
    return (layout()->minimumSize());
}



void SARibbonPannel::setExpanding(bool isExpanding)
{
    setSizePolicy(isExpanding ? QSizePolicy::Expanding : QSizePolicy::Preferred
        , QSizePolicy::Fixed);
}


bool SARibbonPannel::isExpanding() const
{
    QSizePolicy sp = sizePolicy();

    return (sp.horizontalPolicy() == QSizePolicy::Expanding);
}


void SARibbonPannel::resetLayout(PannelLayoutMode newmode)
{
    QGridLayout *layout = m_d->m_gridLayout;
    int c = layout->count();
    struct _tmp_layoutinfo {
        QWidget *	w;
        int		index;
        int		row;
        int		column;
        int		rowSpan;
        int		columnSpan;
    };
    QList<_tmp_layoutinfo> oldlayoutinfo;

    for (int i = 0; i < c; ++i)
    {
        _tmp_layoutinfo s;
        s.w = layout->itemAt(i)->widget();
        s.index = i;
        //获取当前layout的信息
        layout->getItemPosition(i, &s.row, &s.column, &s.rowSpan, &s.columnSpan);
        oldlayoutinfo.append(s);
    }
    //删除布局的所有内容
    m_d->createLayout();
    layout = m_d->m_gridLayout;
    //重新布局
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
    qDebug() << layout->count();
#endif
    if (newmode == ThreeRowMode) {
        //从2行变到3行
        for (_tmp_layoutinfo& info : oldlayoutinfo)
        {
            if (6 == info.rowSpan) {
                //说明占满行
                addLargeWidget(info.w);
                if (qobject_cast<SARibbonToolButton *>(info.w)) {
                    info.w->setFixedHeight(78);
                }
            }else {
                //原来是2行布局，正常add到pannel
                addSmallWidget(info.w);
                if (qobject_cast<SARibbonToolButton *>(info.w)) {
                    info.w->setFixedHeight(24);
                }
            }
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
            _tmp_layoutinfo ttt;
            layout->getItemPosition(info.index, &ttt.row, &ttt.column, &ttt.rowSpan, &ttt.columnSpan);
            qDebug()	<< "info[" << info.index
                    << "] r:" << info.row
                    << " c:" << info.column
                    << " rspan:" << info.rowSpan
                    << " colspan:"<<info.columnSpan
                    << " --> r:" << ttt.row
                    << " c:" << ttt.column
                    << " rspan:" << ttt.rowSpan
                    << " colspan:"<<ttt.columnSpan;
#endif
        }
    }else {
        //从3行变到2行
        for (_tmp_layoutinfo& info : oldlayoutinfo)
        {
            if (6 == info.rowSpan) {
                //说明占满行
                addLargeWidget(info.w);
                //两行的最高不超过64
                if (qobject_cast<SARibbonToolButton *>(info.w)) {
                    info.w->setFixedHeight(64);
                }
            }else {
                //原来是3行布局，正常add到pannel
                addSmallWidget(info.w);
                if (qobject_cast<SARibbonToolButton *>(info.w)) {
                    info.w->setFixedHeight(28);
                }
            }
#ifdef SA_RIBBON_DEBUG_HELP_DRAW
            _tmp_layoutinfo ttt;
            layout->getItemPosition(info.index, &ttt.row, &ttt.column, &ttt.rowSpan, &ttt.columnSpan);
            qDebug()	<< "info[" << info.index
                    << "] r:" << info.row
                    << " c:" << info.column
                    << " rspan:" << info.rowSpan
                    << " colspan:"<<info.columnSpan
                    << " --> r:" << ttt.row
                    << " c:" << ttt.column
                    << " rspan:" << ttt.rowSpan
                    << " colspan:"<<ttt.columnSpan;
#endif
        }
    }
}


/**
 * @brief 重置大按钮的类型
 */
void SARibbonPannel::resetLargeToolButtonStyle()
{
    QList<SARibbonToolButton *> btns = toolButtons();

    for (SARibbonToolButton *b : btns)
    {
        if (nullptr == b || (SARibbonToolButton::LargeButton != b->buttonType())) {
            continue;
        }
        if (ThreeRowMode == pannelLayoutMode()) {
            if (SARibbonToolButton::Normal != b->largeButtonType()) {
                b->setLargeButtonType(SARibbonToolButton::Normal);
            }
        }else{
            if (SARibbonToolButton::Lite != b->largeButtonType()) {
                b->setLargeButtonType(SARibbonToolButton::Lite);
            }
        }
    }
}


void SARibbonPannel::resizeEvent(QResizeEvent *event)
{
    //! 1.移动操作按钮到角落
    if (m_d->m_optionActionButton) {
        if(ThreeRowMode == pannelLayoutMode()){
            m_d->m_optionActionButton->move(width() - m_d->m_optionActionButton->width() - 2
                , height()-RibbonSubElementStyleOpt.pannelTitleHeight
                +(RibbonSubElementStyleOpt.pannelTitleHeight-m_d->m_optionActionButton->height())/2);
        }else{
            m_d->m_optionActionButton->move(width() - m_d->m_optionActionButton->width()
                , height()-m_d->m_optionActionButton->height());
        }
    }
    //! 2.resize后，重新设置分割线的高度
    //! 由于分割线在布局中，只要分割线足够高就可以，不需要重新设置
    return (QWidget::resizeEvent(event));
}

/**
 * @brief 处理action的事件
 * @param e
 */
void SARibbonPannel::actionEvent(QActionEvent *e)
{
    QAction *action = e->action();
    switch (e->type()) {
    case QEvent::ActionChanged:

        break;
    }
}





