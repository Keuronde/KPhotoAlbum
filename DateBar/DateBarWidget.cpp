/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "DateBarWidget.h"
#include <qdatetime.h>
#include <qpainter.h>
#include <qfontmetrics.h>
//Added by qt3to4:
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include "ViewHandler.h"
#include <qtoolbutton.h>
#include <q3popupmenu.h>
#include <qaction.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <math.h>
#include <klocale.h>
#include "Settings/SettingsData.h"
#include <qtooltip.h>
#include <KIcon>

const int borderAboveHistogram = 4;
const int borderArroundWidget = 0;
const int buttonWidth = 22;
const int arrowLength = 20;

/**
 * \namespace DateBar
 * \brief The date bar at the bottom of the main window
 */

/**
 * \class DateBar::DateBarWidget
 * \brief This class represents the date bar at the bottom of the main window.
 *
 * The mouse interaction is handled by the classes which inherits \ref DateBar::MouseHandler, while the logic for
 * deciding the length (in minutes, hours, days, etc) are handled by subclasses of \ref DateBar::ViewHandler.
 */

DateBar::DateBarWidget::DateBarWidget( QWidget* parent, const char* name )
    :QWidget( parent ), _currentHandler( &_yearViewHandler ),_tp(YearView), _currentMouseHandler(0),
     _currentDate( QDateTime::currentDateTime() ),_includeFuzzyCounts( true ), _contextMenu(0),
     _showResolutionIndicator( true )
{
    setObjectName( name );

    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );

    _barWidth = Settings::SettingsData::instance()->histogramSize().width();
    _barHeight = Settings::SettingsData::instance()->histogramSize().height();
    _rightArrow = new QToolButton( this );
    _rightArrow->setArrowType( Qt::RightArrow );
    connect( _rightArrow, SIGNAL( clicked() ), this, SLOT( scrollRight() ) );

    _leftArrow = new QToolButton( this );
    _leftArrow->setArrowType( Qt::LeftArrow );

    connect( _leftArrow, SIGNAL( clicked() ), this, SLOT( scrollLeft() ) );

    _zoomIn = new QToolButton( this );
    _zoomIn->setIcon( KIcon( QString::fromLatin1( "zoom-in" ) ) );
    connect( _zoomIn, SIGNAL( clicked() ), this, SLOT( zoomIn() ) );
    connect( this, SIGNAL(canZoomIn(bool)), _zoomIn, SLOT( setEnabled( bool ) ) );

    _zoomOut = new QToolButton( this );
    _zoomOut->setIcon(  KIcon( QString::fromLatin1( "zoom-out" ) ) );
    connect( _zoomOut, SIGNAL( clicked() ), this, SLOT( zoomOut() ) );
    connect( this, SIGNAL(canZoomOut(bool)), _zoomOut, SLOT( setEnabled( bool ) ) );

    _cancelSelection = new QToolButton( this );
    _cancelSelection->setIcon( KIcon( QString::fromLatin1( "dialog-close" ) ) );
    connect( _cancelSelection, SIGNAL( clicked() ), this, SLOT( clearSelection() ) );
    _cancelSelection->setEnabled( false );
    _cancelSelection->setToolTip( i18n("Widen selection to include all images and videos again") );

    placeAndSizeButtons();

    _focusItemDragHandler = new FocusItemDragHandler( this );
    _barDragHandler = new BarDragHandler( this );
    _selectionHandler = new SelectionHandler( this );

}

QSize DateBar::DateBarWidget::sizeHint() const
{
    int height = qMax( dateAreaGeometry().bottom() + borderArroundWidget,
                       _barHeight+ buttonWidth + 2* borderArroundWidget + 7 );
    return QSize( 800, height );
}

QSize DateBar::DateBarWidget::minimumSizeHint() const
{
     int height = qMax( dateAreaGeometry().bottom() + borderArroundWidget,
                        _barHeight + buttonWidth + 2* borderArroundWidget + 7 );
     return QSize( 200, height );
}

void DateBar::DateBarWidget::paintEvent( QPaintEvent* /*event*/ )
{
    QPainter painter( this );
    painter.drawPixmap( 0,0, _buffer );
}

void DateBar::DateBarWidget::redraw()
{
    if ( _buffer.isNull() )
        return;

    if (_dates.isNull() )
        return;

    QPainter p( &_buffer );
    p.setRenderHint( QPainter::Antialiasing );
    p.setFont( font() );

    // Fill with background pixels
    p.save();
    p.setPen( Qt::NoPen );
    p.setBrush( palette().brush( QPalette::Background ) );
    p.drawRect( rect() );

    // Draw the area with histograms
    QRect barArea = barAreaGeometry();

    p.setPen( palette().color( QPalette::Dark ) );
    p.setBrush( palette().brush( QPalette::Base ) );
    p.drawRect( barArea );
    p.restore();

    _currentHandler->init( dateForUnit( -_currentUnit, _currentDate ) );

    int right;
    drawResolutionIndicator( p, &right );
    QRect rect = dateAreaGeometry();
    rect.setRight( right );
    rect.setLeft( rect.left() + buttonWidth + 2 );

    drawTickMarks( p, rect );
    drawHistograms( p );
    drawFocusRectagle( p );
    updateArrowState();
    repaint();
}

void DateBar::DateBarWidget::resizeEvent( QResizeEvent* event )
{
    placeAndSizeButtons();
    _buffer = QPixmap( event->size() );
    _currentUnit = numberOfUnits()/2;
    redraw();
}

void DateBar::DateBarWidget::drawTickMarks( QPainter& p, const QRect& textRect )
{
    QRect rect = tickMarkGeometry();
    p.save();
    p.setPen( QPen( palette().color( QPalette::Text) , 1 ) );

    QFont f( font() );
    QFontMetrics fm(f);
    int fontHeight = fm.height();
    int unit = 0;
    QRect clip = rect;
    clip.setHeight( rect.height() + 2 + fontHeight );
    clip.setLeft( clip.left() + 2 );
    clip.setRight( clip.right() -2 );
    p.setClipRect( clip );

    for ( int x = rect.x(); x < rect.right(); x+=_barWidth, unit += 1 ) {
        // draw selection indication
        p.save();
        p.setPen( Qt::NoPen );
        p.setBrush( palette().brush( QPalette::Highlight ) );
        QDateTime date = dateForUnit( unit );
        if ( isUnitSelected( unit ) )
            p.drawRect( QRect( x, rect.top(), _barWidth, rect.height() ) );
        p.restore();

        // draw tickmarks
        int h = rect.height();
        if ( _currentHandler->isMajorUnit( unit ) ) {
            QString text = _currentHandler->text( unit );
            int w = fm.width( text );
            p.setFont( f );
            if ( textRect.right() >  x + w/2 && textRect.left() < x - w/2)
                p.drawText( x - w/2, textRect.top(), w, fontHeight, Qt::TextSingleLine, text );
        }
        else if ( _currentHandler->isMidUnit( unit ) )
            h = (int) ( 2.0/3*rect.height());
        else
            h = (int) ( 1.0/3*rect.height());

        p.drawLine( x, rect.top(), x, rect.top() + h );
    }

    p.restore();
}

void DateBar::DateBarWidget::setViewType( ViewType tp )
{
    switch ( tp ) {
    case DecadeView: _currentHandler = &_decadeViewHandler; break;
    case YearView: _currentHandler = &_yearViewHandler; break;
    case MonthView: _currentHandler = &_monthViewHandler; break;
    case WeekView: _currentHandler = &_weekViewHandler; break;
    case DayView: _currentHandler = &_dayViewHandler; break;
    case HourView: _currentHandler = &_hourViewHandler; break;
    }
    redraw();
    _tp = tp;
}

void DateBar::DateBarWidget::setDate( const QDateTime& date )
{
    _currentDate = date;
    if ( hasSelection() ) {
        if ( currentSelection().start() > _currentDate )
            _currentDate = currentSelection().start();
        if ( currentSelection().end() < _currentDate )
            _currentDate = currentSelection().end();
    }

    if ( unitForDate( _currentDate ) != -1 )
        _currentUnit = unitForDate( _currentDate );

    redraw();
}

void DateBar::DateBarWidget::setImageDateCollection( const KSharedPtr<DB::ImageDateCollection>& dates )
{
    _dates = dates;
    redraw();
}

void DateBar::DateBarWidget::drawHistograms( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    p.setClipping( true );
    p.setClipRect( rect );
    p.setPen( Qt::NoPen );

    int unit = 0;
    int max = 0;
    for ( int x = rect.x(); x + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        DB::ImageCount count = _dates->count( DB::ImageDate( dateForUnit(unit), dateForUnit(unit+1).addSecs(-1) ) );
        int cnt = count._exact;
        if ( _includeFuzzyCounts )
            cnt += count._rangeMatch;
        max = qMax( max, cnt  );
    }

    unit = 0;
    for ( int x = rect.x(); x  + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        DB::ImageCount count = _dates->count( DB::ImageDate( dateForUnit(unit), dateForUnit(unit+1).addSecs(-1) ) );
        int exact = 0;
        if ( max != 0 )
            exact = (int) ((double) (rect.height()-2) * count._exact / max );
        int range = 0;
        if ( _includeFuzzyCounts && max != 0 )
            range = (int) ((double) (rect.height()-2) * count._rangeMatch / max );

        Qt::BrushStyle style = Qt::SolidPattern;
        if ( !isUnitSelected( unit ) && hasSelection() )
            style= Qt::Dense5Pattern;

        p.setBrush( QBrush( Qt::yellow, style ) );
        p.drawRect( x+1, rect.bottom()-range, _barWidth-2, range );
        p.setBrush( QBrush( Qt::green, style ) );
        p.drawRect( x+1, rect.bottom()-range-exact, _barWidth-2, exact );

        // calculate the font size for the largest number.
        QFont f = font();
        bool found = false;
        for ( int i = f.pointSize(); i >= 6; i-=2 ) {
            f.setPointSize( i );
            int w = QFontMetrics(f).width( QString::number( max ) );
            if ( w < rect.height() - 6 ) {
                p.setFont(f);
                found = true;
                break;
            }
        }

        // draw the numbers
        int tot = count._exact;
        if ( _includeFuzzyCounts )
            tot += count._rangeMatch;
        p.save();
        p.translate( x+_barWidth-3, rect.bottom()-2 );
        p.rotate( -90 );
        int w = QFontMetrics(f).width( QString::number( tot ) );
        if ( w < exact+range-2 ) {
            p.drawText( 0,0, QString::number( tot ) );
        }
        p.restore();
    }

    p.restore();
}

void DateBar::DateBarWidget::scrollLeft()
{
    scroll( -1 );
}

void DateBar::DateBarWidget::scrollRight()
{
    scroll( 1 );
}

void DateBar::DateBarWidget::scroll( int units )
{
    _currentDate = dateForUnit( units, _currentDate );
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::drawFocusRectagle( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    int x = rect.left() + _currentUnit*_barWidth;
    QRect inner( QPoint(x-1, borderAboveHistogram),
                 QPoint( x + _barWidth, borderAboveHistogram + _barHeight - 1 ) );

    p.setPen( QPen( palette().color( QPalette::Dark ), 1 ) );

    // Inner rect
    p.drawRect( inner );
    QRect outer = inner;
    outer.adjust( -2, -2, 2, 2 );

    // Outer rect
    QRegion region = outer;
    region -= inner;
    p.setClipping( true );
    p.setClipRegion( region );

    QColor col = Qt::gray;
    if ( !hasFocus() )
        col = Qt::white;

    p.setBrush( col );
    p.setPen( col );
    p.drawRect( outer );

    // Shadow below
    QRect shadow = outer;
    shadow.adjust( -1,-1, 1, 1 );
    region = shadow;
    region -= outer;
    p.setPen( palette().color( QPalette::Shadow ) );
    p.setClipRegion( region );
    p.drawRect( shadow );

    // Light above
    QRect hide = shadow;
    hide.translate( 1, 1 );
    region = shadow;
    region -= hide;
    p.setPen( palette().color( QPalette::Light ) );
    p.setClipRegion( region );
    p.drawRect( shadow );

    p.restore();
}

void DateBar::DateBarWidget::zoomIn()
{
    if ( _tp == HourView )
        return;
    zoom(+1);
}

void DateBar::DateBarWidget::zoomOut()
{
    if ( _tp == DecadeView )
        return;
    zoom(-1);
}

void DateBar::DateBarWidget::zoom( int factor )
{
    ViewType tp = (ViewType) (_tp+factor);
    setViewType( tp );
    emit canZoomIn( tp != HourView );
    emit canZoomOut( tp != DecadeView );
}

void DateBar::DateBarWidget::mousePressEvent( QMouseEvent* event )
{
    if ( (event->button() & Qt::LeftButton) == 0 ||  event->x() > barAreaGeometry().right() || event->x() < barAreaGeometry().left() )
        return;

    if ( event->modifiers() & Qt::ControlModifier ) {
        _currentMouseHandler = _barDragHandler;
    }
    else {
        bool onBar = event->y() > barAreaGeometry().bottom();
        if ( onBar )
            _currentMouseHandler = _selectionHandler;
        else {
            _currentMouseHandler= _focusItemDragHandler;
        }
    }
    _currentMouseHandler->mousePressEvent( event->x() );
    _cancelSelection->setEnabled( hasSelection() );
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
    showStatusBarTip( event->pos() );
    redraw();
}

void DateBar::DateBarWidget::mouseReleaseEvent( QMouseEvent* )
{
    if ( _currentMouseHandler == 0 )
        return;

    _currentMouseHandler->endAutoScroll();
    _currentMouseHandler->mouseReleaseEvent();
    _currentMouseHandler = 0;
}

void DateBar::DateBarWidget::mouseMoveEvent( QMouseEvent* event )
{
    if ( _currentMouseHandler == 0)
        return;

    showStatusBarTip( event->pos() );

    if ( (event->buttons() & Qt::LeftButton) == 0 )
        return;

    _currentMouseHandler->endAutoScroll();
    _currentMouseHandler->mouseMoveEvent( event->pos().x() );
}

QRect DateBar::DateBarWidget::barAreaGeometry() const
{
    QRect barArea;
    barArea.setTopLeft( QPoint( borderArroundWidget, borderAboveHistogram ) );
    barArea.setRight( width() - borderArroundWidget - 2 * buttonWidth - 2*3 ); // 2 pixels between button and bar + 1 pixel as the pen is one pixel
    barArea.setHeight( _barHeight );
    return barArea;
}

int DateBar::DateBarWidget::numberOfUnits() const
{
    return barAreaGeometry().width() / _barWidth -1 ;
}

void DateBar::DateBarWidget::setHistogramBarSize( const QSize& size )
{
    _barWidth = size.width();
    _barHeight = size.height();
    _currentUnit = numberOfUnits()/2;
    Q_ASSERT( parentWidget() );
    updateGeometry();
    Q_ASSERT( parentWidget() );
    placeAndSizeButtons();
    redraw();
}

void DateBar::DateBarWidget::setIncludeFuzzyCounts( bool b )
{
    _includeFuzzyCounts = b;
    redraw();
    if ( hasSelection() )
        emitRangeSelection( _selectionHandler->dateRange() );

    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

DB::ImageDate DateBar::DateBarWidget::rangeAt( const QPoint& p )
{
    int unit = (p.x() - barAreaGeometry().x())/ _barWidth;
    return DB::ImageDate( dateForUnit( unit ), dateForUnit(unit+1) );
}

bool DateBar::DateBarWidget::includeFuzzyCounts() const
{
    return _includeFuzzyCounts;
}

void DateBar::DateBarWidget::contextMenuEvent( QContextMenuEvent* event )
{
    if ( !_contextMenu ) {
        _contextMenu = new QMenu( this );
        QAction* action = new QAction( i18n("Show Ranges"), this );
        action->setCheckable( true );
        _contextMenu->addAction(action);
        action->setChecked( _includeFuzzyCounts );
        connect( action, SIGNAL( toggled( bool ) ), this, SLOT( setIncludeFuzzyCounts( bool ) ) );

        action = new QAction( i18n("Show Resolution Indicator"), this );
        action->setCheckable( true );
        _contextMenu->addAction(action);
        action->setChecked( _showResolutionIndicator );
        connect( action, SIGNAL( toggled( bool ) ), this, SLOT( setShowResolutionIndicator( bool ) ) );
    }

    _contextMenu->exec( event->globalPos());
    event->setAccepted(true);
}

QRect DateBar::DateBarWidget::tickMarkGeometry() const
{
    QRect rect;
    rect.setTopLeft( barAreaGeometry().bottomLeft() );
    rect.setWidth( barAreaGeometry().width() );
    rect.setHeight( 12 );
    return rect;
}

void DateBar::DateBarWidget::drawResolutionIndicator( QPainter& p, int* leftEdge )
{
    QRect rect = dateAreaGeometry();

    // For real small bars, we do not want to show the resolution.
    if ( rect.width() < 400 || !_showResolutionIndicator ) {
        *leftEdge = rect.right();
        return;
    }

    QString text = _currentHandler->unitText();
    int textWidth = QFontMetrics( font() ).width( text );
    int height = QFontMetrics( font() ).height();

    int endUnitPos = rect.right() - textWidth - arrowLength - 3;
    // Round to nearest unit mark
    endUnitPos = ( (endUnitPos-rect.left()) / _barWidth) * _barWidth + rect.left();
    int startUnitPos = endUnitPos - _barWidth;
    int midLine = rect.top() + height / 2;

    p.save();
    p.setPen( Qt::red );

    // draw arrows
    drawArrow( p, QPoint( startUnitPos - arrowLength, midLine ), QPoint( startUnitPos, midLine ) );
    drawArrow( p, QPoint( endUnitPos + arrowLength, midLine ), QPoint( endUnitPos, midLine ) );
    p.drawLine( startUnitPos, rect.top(), startUnitPos, rect.top()+height );
    p.drawLine( endUnitPos, rect.top(), endUnitPos, rect.top()+height );

    // draw text
    QFontMetrics fm( font() );
    p.drawText( endUnitPos + arrowLength + 3, rect.top(), fm.width(text), fm.height(), Qt::TextSingleLine, text );
    p.restore();

    *leftEdge = startUnitPos - arrowLength - 3;
}

QRect DateBar::DateBarWidget::dateAreaGeometry() const
{
    QRect rect = tickMarkGeometry();
    rect.setTop( rect.bottom() + 2 );
    rect.setHeight( QFontMetrics( font() ).height() );
    return rect;
}

void DateBar::DateBarWidget::drawArrow( QPainter& p, const QPoint& start, const QPoint& end )
{
    p.save();
    p.drawLine( start, end );

    QPoint diff = QPoint( end.x() - start.x(), end.y() - start.y() );
    double dx = diff.x();
    double dy = diff.y();

    if ( dx != 0 || dy != 0 ) {
        if( dy < 0 ) dx = -dx;
        double angle = acos(dx/sqrt( dx*dx+dy*dy ))*180./M_PI;
        if( dy < 0 ) angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        p.translate( end.x(), end.y() );
        p.rotate( angle );
        p.drawLine( QPoint(0,0), QPoint( 10,0 ) );

        p.rotate( 30 );
        p.drawLine( QPoint(0,0), QPoint( 10,0 ) );
    }

    p.restore();

}

void DateBar::DateBarWidget::setShowResolutionIndicator( bool b )
{
    _showResolutionIndicator = b;
    redraw();
}

void DateBar::DateBarWidget::updateArrowState()
{
    _leftArrow->setEnabled( _dates->lowerLimit() <= dateForUnit( 0 ) );
    _rightArrow->setEnabled( _dates->upperLimit() > dateForUnit( numberOfUnits() ) );
}

DB::ImageDate DateBar::DateBarWidget::currentDateRange() const
{
    return DB::ImageDate( dateForUnit( _currentUnit ), dateForUnit( _currentUnit+1 ) );
}

void DateBar::DateBarWidget::showStatusBarTip( const QPoint& pos )
{
    DB::ImageDate range = rangeAt( pos );
    DB::ImageCount count = _dates->count( range );

    QString cnt;
    if ( count._rangeMatch != 0 && includeFuzzyCounts())
        cnt = i18n("%1 exact + %2 ranges = %3 total", count._exact , count._rangeMatch , count._exact + count._rangeMatch );
    else
        cnt = i18n("%1 images/videos", count._exact );

    QString res = i18n("%1 to %2  %3",range.start().toString(),range.end().toString(),
                  cnt);

    static QString lastTip = QString::null;
    if ( lastTip != res )
        emit toolTipInfo( res );
    lastTip = res;
}

void DateBar::DateBarWidget::placeAndSizeButtons()
{
    _zoomIn->setFixedSize( buttonWidth, buttonWidth );
    _zoomOut->setFixedSize( buttonWidth, buttonWidth );
    _rightArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );
    _leftArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );

    _rightArrow->move( size().width() - _rightArrow->width() - borderArroundWidget, borderAboveHistogram );
    _leftArrow->move( _rightArrow->pos().x() - _leftArrow->width() -2 , borderAboveHistogram );

    int x = _leftArrow->pos().x();
    int y = height() - buttonWidth;
    _zoomOut->move( x, y );

    x = _rightArrow->pos().x();
    _zoomIn->move(x, y );


    _cancelSelection->setFixedSize( buttonWidth, buttonWidth );
    _cancelSelection->move( 0, y );
}

void DateBar::DateBarWidget::keyPressEvent( QKeyEvent* event )
{
    int offset = 0;
    if ( event->key() == Qt::Key_Plus ) {
        if ( _tp != HourView )
            zoom(1);
        return;
    }
    if ( event->key() == Qt::Key_Minus ) {
        if ( _tp != DecadeView )
            zoom( -1 );
        return;
    }

    if ( event->key() == Qt::Key_Left )
        offset = -1;
    else if ( event->key() == Qt::Key_Right )
        offset = 1;
    else if ( event->key() == Qt::Key_PageDown )
        offset = -10;
    else if ( event->key() == Qt::Key_PageUp )
        offset = 10;
    else
        return;

    QDateTime newDate =dateForUnit( offset, _currentDate );
    if ( (offset < 0 && newDate >= _dates->lowerLimit()) ||
         ( offset > 0 && newDate <= _dates->upperLimit() ) ) {
        _currentDate = newDate;
        _currentUnit += offset;
        if ( _currentUnit < 0 )
            _currentUnit = 0;
        if ( _currentUnit > numberOfUnits() )
            _currentUnit = numberOfUnits();

        if ( ! currentSelection().includes( _currentDate ) )
            clearSelection();
    }
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::focusInEvent( QFocusEvent* )
{
    redraw();
}

void DateBar::DateBarWidget::focusOutEvent( QFocusEvent* )
{
    redraw();
}


int DateBar::DateBarWidget::unitAtPos( int x ) const
{
    return ( x  - barAreaGeometry().left() )/_barWidth;
}

QDateTime DateBar::DateBarWidget::dateForUnit( int unit, const QDateTime& offset ) const
{
    return _currentHandler->date( unit, offset );
}

bool DateBar::DateBarWidget::isUnitSelected( int unit ) const
{
    QDateTime minDate = _selectionHandler->min();
    QDateTime maxDate = _selectionHandler->max();
    QDateTime date = dateForUnit( unit );
    return ( minDate <= date && date < maxDate && !minDate.isNull() );
}

bool DateBar::DateBarWidget::hasSelection() const
{
    return !_selectionHandler->min().isNull();
}

DB::ImageDate DateBar::DateBarWidget::currentSelection() const
{
    return DB::ImageDate(_selectionHandler->min(), _selectionHandler->max() );
}

void DateBar::DateBarWidget::clearSelection()
{
    if ( _selectionHandler->hasSelection() ) {
        _selectionHandler->clearSelection();
        emit dateRangeCleared();
        redraw();
    }
    _cancelSelection->setEnabled( false );
}

void DateBar::DateBarWidget::emitRangeSelection( const DB::ImageDate&  range )
{
    emit dateRangeChange( range );
}

int DateBar::DateBarWidget::unitForDate( const QDateTime& date ) const
{
    for ( int unit = 0; unit < numberOfUnits(); ++unit ) {
        if ( _currentHandler->date( unit ) <= date && date < _currentHandler->date( unit +1 ) )
            return unit;
    }
    return -1;
}

void DateBar::DateBarWidget::emitDateSelected()
{
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::wheelEvent( QWheelEvent * e )
{
    if ( e->modifiers() & Qt::ControlModifier ) {
        if ( e->delta() > 0 )
            zoomIn();
        else
            zoomOut();
        return;
    }
    if ( e->delta() > 0 )
        scroll(1);
    else
        scroll(-1);
}

#include "DateBarWidget.moc"
