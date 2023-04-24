﻿#include "SAColorGridWidget.h"
#include "SAColorToolButton.h"
#include <QGridLayout>
#include <cmath>
#include <QButtonGroup>
#include <functional>
class SAColorGridWidget::PrivateData
{
    SA_DECLARE_PUBLIC(SAColorGridWidget)
public:
    PrivateData(SAColorGridWidget* p);
    //获取ColorToolButton
    SAColorToolButton* getColorToolButtonAt(int index);
    SAColorToolButton* getColorToolButtonAt(int r, int c);
    void updateGridColor();
    void updateGridColorSize();
    void updateGridColorCheckable();
    void updateGridColorBtns(std::function< void(SAColorToolButton*) > fn);
    void removeAt(int r, int c);
    void setColorAt(const QColor& clr, int r, int c);

public:
    QList< QColor > mColors;
    QGridLayout* mGridLayout { nullptr };
    QButtonGroup* mButtonGroup { nullptr };
    QSize mIconSize { 16, 16 };
    int mColumnCount { 8 };  ///< 列数，行数量会根据列数量来匹配,如果设置-1或者0，说明不限定列数量，这样会只有一行
    bool mColorCheckable;    ///<设置颜色是否是checkable
};

SAColorGridWidget::PrivateData::PrivateData(SAColorGridWidget* p) : q_ptr(p)
{
    mGridLayout = new QGridLayout(q_ptr);
    q_ptr->setLayout(mGridLayout);
    mGridLayout->setSpacing(4);
    mButtonGroup = new QButtonGroup(q_ptr);
    mButtonGroup->setExclusive(true);
}

SAColorToolButton* SAColorGridWidget::PrivateData::getColorToolButtonAt(int index)
{
    QLayoutItem* item = mGridLayout->itemAt(index);
    if (nullptr == item) {
        return nullptr;
    }
    return qobject_cast< SAColorToolButton* >(item->widget());
}

SAColorToolButton* SAColorGridWidget::PrivateData::getColorToolButtonAt(int r, int c)
{
    QLayoutItem* item = mGridLayout->itemAtPosition(r, c);
    if (nullptr == item) {
        return nullptr;
    }
    return qobject_cast< SAColorToolButton* >(item->widget());
}

/**
 * @brief 根据mColors更新布局
 */
void SAColorGridWidget::PrivateData::updateGridColor()
{
    int row = 1;
    int col = mColumnCount;
    if (col <= 0) {
        col = mColors.size();
    } else {
        row = std::ceil(mColors.size() / (float)col);
    }
    int index = 0;
    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < col; ++c) {
            if (index < mColors.size()) {
                setColorAt(mColors[ index ], r, c);
                ++index;
            } else {
                removeAt(r, c);
            }
        }
    }
}

/**
 * @brief 更新colorsize
 */
void SAColorGridWidget::PrivateData::updateGridColorSize()
{
    QSize s = mIconSize;
    updateGridColorBtns([ s ](SAColorToolButton* btn) {
        if (btn) {
            btn->setIconSize(s);
        }
    });
}

void SAColorGridWidget::PrivateData::updateGridColorCheckable()
{
    bool v = mColorCheckable;
    updateGridColorBtns([ v ](SAColorToolButton* btn) {
        if (btn) {
            btn->setCheckable(v);
        }
    });
}

/**
 * @brief 遍历所有的button
 * @param fn
 */
void SAColorGridWidget::PrivateData::updateGridColorBtns(std::function< void(SAColorToolButton*) > fn)
{
    int cnt = mGridLayout->count();
    for (int i = 0; i < cnt; ++i) {
        SAColorToolButton* tl = getColorToolButtonAt(i);
        fn(tl);
    }
}

/**
 * @brief 删除网格窗口
 * @param r
 * @param c
 */
void SAColorGridWidget::PrivateData::removeAt(int r, int c)
{
    QLayoutItem* item = mGridLayout->itemAtPosition(r, c);
    if (item) {
        QWidget* w = item->widget();
        mGridLayout->removeItem(item);
        delete item;
        w->deleteLater();
    }
}

void SAColorGridWidget::PrivateData::setColorAt(const QColor& clr, int r, int c)
{
    QLayoutItem* item = mGridLayout->itemAtPosition(r, c);
    if (item) {
        SAColorToolButton* tl = qobject_cast< SAColorToolButton* >(item->widget());
        if (tl) {
            tl->setColor(clr);
        }
    } else {
        SAColorToolButton* tl = new SAColorToolButton(q_ptr);
        tl->setToolButtonStyle(Qt::ToolButtonIconOnly);
        tl->setIconSize(mIconSize);
        tl->setMargins(QMargins(4, 4, 4, 4));
        tl->setColor(clr);
        tl->setCheckable(true);
        tl->setAutoRaise(true);
        mButtonGroup->addButton(tl, r + c);
        mGridLayout->addWidget(tl, r, c);
    }
}

//==============================================================
// SAColorGridWidget
//==============================================================

SAColorGridWidget::SAColorGridWidget(QWidget* par) : QWidget(par), d_ptr(new SAColorGridWidget::PrivateData(this))
{
    connect(d_ptr->mButtonGroup, QOverload< QAbstractButton* >::of(&QButtonGroup::buttonClicked), this, &SAColorGridWidget::onButtonClicked);
    connect(d_ptr->mButtonGroup, QOverload< QAbstractButton* >::of(&QButtonGroup::buttonPressed), this, &SAColorGridWidget::onButtonPressed);
    connect(d_ptr->mButtonGroup, QOverload< QAbstractButton* >::of(&QButtonGroup::buttonReleased), this, &SAColorGridWidget::onButtonReleased);
    connect(d_ptr->mButtonGroup, QOverload< QAbstractButton*, bool >::of(&QButtonGroup::buttonToggled), this, &SAColorGridWidget::onButtonToggled);
}

SAColorGridWidget::~SAColorGridWidget()
{
}

void SAColorGridWidget::setColumnCount(int c)
{
    d_ptr->mColumnCount = c;
}

/**
 * @brief 设置颜色列表
 * @param c
 */
void SAColorGridWidget::setColorList(const QList< QColor >& cls)
{
    d_ptr->mColors = cls;
    d_ptr->updateGridColor();
}

/**
 * @brief 获取颜色列表
 * @return
 */
QList< QColor > SAColorGridWidget::getColorList() const
{
    return d_ptr->mColors;
}

/**
 * @brief 获取间隔
 * @return
 */
int SAColorGridWidget::getSpacing() const
{
    return d_ptr->mGridLayout->spacing();
}

/**
 * @brief 设置间隔
 * @param v
 */
void SAColorGridWidget::setSpacing(int v)
{
    d_ptr->mGridLayout->setSpacing(v);
}

/**
 * @brief 获取颜色的数量
 * @return
 */
int SAColorGridWidget::getColorCount() const
{
    return d_ptr->mColors.size();
}

/**
 * @brief 设置图标 size
 * @return
 */
void SAColorGridWidget::setColorIconSize(const QSize& s)
{
    d_ptr->mIconSize = s;
    d_ptr->updateGridColorSize();
}

/**
 * @brief 获取图标 size
 * @return
 */
QSize SAColorGridWidget::getColorIconSize() const
{
    return d_ptr->mIconSize;
}

/**
 * @brief 设置颜色是否是checkable
 *
 * checkable的颜色按钮是可checked的
 * @param on
 */
void SAColorGridWidget::setColorCheckable(bool on)
{
    d_ptr->mColorCheckable = on;
    d_ptr->updateGridColorCheckable();
}

/**
 * @brief 颜色是否是checkable
 * @return
 */
bool SAColorGridWidget::isColorCheckable() const
{
    return d_ptr->mColorCheckable;
}

/**
 * @brief 获取当前选中的颜色
 * @return
 */
QColor SAColorGridWidget::getCurrentCheckedColor() const
{
    QAbstractButton* btn = d_ptr->mButtonGroup->checkedButton();
    if (nullptr == btn) {
        return QColor();
    }
    SAColorToolButton* t = qobject_cast< SAColorToolButton* >(btn);
    if (nullptr == t) {
        return QColor();
    }
    return t->getColor();
}

/**
 * @brief 获取index对应的colorbutton
 * @param index
 * @return 如果没有返回nullptr
 */
SAColorToolButton* SAColorGridWidget::getColorButton(int index) const
{
    return d_ptr->getColorToolButtonAt(index);
}

void SAColorGridWidget::onButtonClicked(QAbstractButton* btn)
{
    SAColorToolButton* t = qobject_cast< SAColorToolButton* >(btn);
    if (t) {
        emit colorClicked(t->getColor());
    }
}

void SAColorGridWidget::onButtonPressed(QAbstractButton* btn)
{
    SAColorToolButton* t = qobject_cast< SAColorToolButton* >(btn);
    if (t) {
        emit colorPressed(t->getColor());
    }
}

void SAColorGridWidget::onButtonToggled(QAbstractButton* btn, bool on)
{
    SAColorToolButton* t = qobject_cast< SAColorToolButton* >(btn);
    if (t) {
        emit colorToggled(t->getColor(), on);
    }
}

void SAColorGridWidget::onButtonReleased(QAbstractButton* btn)
{
    SAColorToolButton* t = qobject_cast< SAColorToolButton* >(btn);
    if (t) {
        emit colorReleased(t->getColor());
    }
}

namespace SA
{

QList< QColor > getStandardColorList()
{
    static QList< QColor > s_standardColorList({ QColor(192, 0, 0),
                                                 QColor(255, 0, 0),
                                                 QColor(255, 192, 0),
                                                 QColor(192, 0, 0),
                                                 QColor(255, 255, 0),
                                                 QColor(146, 208, 80),
                                                 QColor(0, 176, 80),
                                                 QColor(0, 176, 240),
                                                 QColor(0, 112, 192),
                                                 QColor(0, 32, 96),
                                                 QColor(112, 48, 160) });
    return s_standardColorList;
}
}
