#ifndef __GRAPH_SCREEN_H__
#define __GRAPH_SCREEN_H__

#include "graphics_item.h"
#include "../utilities/ring_buffer.h"

//
namespace LT
{
    enum GraphType : uint8_t
    {
        ScatterGraph= 0,
        BarGraph = 1,
        LineGraph = 2
    };
}


template <typename T>
struct DataPoint {
    T x;
    T y;
    DataPoint(T key = 0, T value = 0) : x(key), y(value){}
};

//template <uint8_t POINTS, typename T>
template <int POINTS, typename T, class TS = typename SelectInteger<POINTS>::type>
class DataSet : public RingBuffer<POINTS, DataPoint<T> > {
    T _y_min = 1;
    T _y_max = 1;
    T _x_min = 1;
    T _x_max = 1;
    
    
    void updateBounds(){
      if(RingBuffer<POINTS, DataPoint<T> >::isEmpty()) return;
      DataPoint<T> p;
      RingBuffer<POINTS, DataPoint<T> >::get(0,&p);
      _x_min = p.x;
      _x_max = p.x;
      _y_min = p.y;
      _y_max = p.y;
      TS i = RingBuffer<POINTS, DataPoint<T> >::count();
      do {
        RingBuffer<POINTS,DataPoint<T> >::get( i - 1, &p );
        if(p.x < _x_min) {
          _x_min = p.x;
        }
        if(p.x > _x_max) {
          _x_max = p.x;
        }
        if(p.y < _y_min) {
          _y_min = p.y;
        }
        if(p.y > _y_max) {
          _y_max = p.y;
        }
      } while ( --i );
    }
    
    public:
    inline T xMax(){return _x_max;}
    inline T yMax(){return _y_max;}
    inline T xMin(){return _x_min;}
    inline T yMin(){return _y_min;}
    
    void push(const DataPoint<T> data, DataPoint<T> *taken) {
       RingBuffer<POINTS,DataPoint<T> >::push(data, taken);
       updateBounds();
    }
    
};

/*!
 * @brief the GraphItem class...
 * 
 * @tparam T 
 * @tparam POINTS 
 * @tparam SelectInteger<POINTS>::type 
 */
template <typename T, int POINTS, class TS = typename SelectInteger<POINTS>::type>
class GraphItem : public GraphicsItem {
    const char* _title = NULL;
    const char* _x_label = NULL;
    const char* _y_label = NULL;
    
    bool _label_last_point = true;
    float _x_scale = 1;
    float _y_scale = 1;
    
    //< store the size of the graph. default values for a 128x64 px oled
    const uint8_t GRAPH_X = 16;
    const uint8_t GRAPH_Y = 8;
    const uint8_t GRAPH_W = 48;
    const uint8_t GRAPH_H = 48;
    
    //RingBuffer<POINTS, DataPoint<T> > dataSet;
    uint8_t _graph_type = 0;
    DataSet<POINTS, T> dataSet;
    
    uint8_t scaleX(T value, float x_scale) {
      return (value - dataSet.xMin()) * x_scale + GRAPH_X;
    }
    uint8_t scaleY(T value, float y_scale) {
      return (value - dataSet.yMin()) * y_scale - GRAPH_Y;
    }
    void updateXScale(T x_range) {
      _x_scale = (float)(GRAPH_W) / (float)(x_range);
    } 
    void updateYScale(T y_range) {
      _y_scale = (float)(GRAPH_H) / (float)(y_range);
    } 

    /*void markCorners(UiContext* context) {
      context->setFontRegular();
      context->display->drawStr(GRAPH_X, GRAPH_Y, "TL");
      context->display->drawStr(GRAPH_X, GRAPH_Y+GRAPH_H, "BL");
      context->display->drawStr(GRAPH_X+GRAPH_W, GRAPH_Y, "TR");
      context->display->drawStr(GRAPH_X+GRAPH_W, GRAPH_Y+GRAPH_H, "BR");
    }*/

    // if x oveflows:
    // 245, 250, 255, 0, 5, 10 ....
    // take the difference between the first and last items in the RingBuffer
    // first (newest) = 10, last (oldest) = 245. Then x-position = 255 + (new-old)
    // oldest (245) - newest (10)
    // 245 - 10, 250 - 10,  255 - 10, 0 - 10  5 - 10  10 - 10
    // 235,      240,       245,      246,    251,      0
    
    void drawScatter(UiContext* context) {
      TS i = dataSet.count();
      uint8_t my, mx;
      DataPoint<T> p;
      do {
        dataSet.get( i - 1, &p );
        mx = scaleX(p.x, _x_scale);
        my = scaleY(p.y, _y_scale);
        context->display->drawPixel( mx, GRAPH_H - my );
      } while( --i );
    }
    void drawLines(UiContext* context) {
      TS i = dataSet.count();
      uint8_t mx1, my1, mx2, my2;
      DataPoint<T> p1, p2;
      do {
        if( i > 1) {
          dataSet.get(i-2,&p1);
          dataSet.get(i-1,&p2);
          mx1 = scaleX(p1.x, _x_scale);
          my1 = scaleY(p1.y, _y_scale);
          mx2 = scaleX(p2.x, _x_scale);
          my2 = scaleY(p2.y, _y_scale);
          context->display->drawLine(mx1, GRAPH_H-my1, mx2, GRAPH_H-my2);
        } 
      } while( --i );
    }
    void drawBars(UiContext* context) {
      TS i = dataSet.count();
      uint8_t my, mx;
      DataPoint<T> p;
      do {
        dataSet.get( i - 1, &p );
        mx = scaleX(p.x, _x_scale);
        my = scaleY(p.y, _y_scale);
        context->display->drawVLine( mx, GRAPH_H - my, my );
      } while( --i );
    }
    
    void drawAxes(UiContext* context) {
        context->display->setDrawColor(1);
        //draw tick at top of y
        context->display->drawHLine(GRAPH_X, GRAPH_Y, 4);
        //draw line along X
        context->display->drawHLine(GRAPH_X, GRAPH_H + GRAPH_Y ,GRAPH_W);
        
        //print x labels
        context->setCurrentFont(context->getFontSmall());
        uint8_t padding_left = ( GRAPH_W - context->display->getStrWidth(_x_label) ) >> 1;
        context->display->setCursor(padding_left + GRAPH_X, GRAPH_H + GRAPH_Y + 8);
        context->display->print(_x_label);
        
        context->display->setCursor(GRAPH_X, GRAPH_H + GRAPH_Y + 8);
        context->display->print(dataSet.xMin(), 2); //DEC?

        context->display->setCursor(GRAPH_X + GRAPH_W - 24, GRAPH_H + GRAPH_Y + 8);
        context->display->print(dataSet.xMax(), 2); //DEC?
        
        //print y labels
        uint8_t padding_top = ( GRAPH_H - context->display->getStrWidth(_y_label) ) >> 1 ;
        context->display->setFontDirection(3);
        context->display->drawStr(GRAPH_X, GRAPH_H + GRAPH_Y - padding_top, _y_label);
        context->display->setFontDirection(0);
        
        context->display->setCursor(GRAPH_X + 1, GRAPH_H + GRAPH_Y);
        context->display->print(dataSet.yMin(), 2); //DEC?
        
        context->display->setCursor(GRAPH_X + 1, GRAPH_Y + 8);
        context->display->print(dataSet.yMax(), 2); //DEC?
    }
  
  public:
  /*!
   * @brief Construct a new Graph Item object
   * 
   * @param parent 
   * @param title 
   * @param x_label 
   * @param y_label 
   * @param x 
   * @param y 
   * @param w 
   * @param h 
   */
    GraphItem(GraphicsItem* parent, const char* title = NULL, 
    const char* x_label = NULL, const char* y_label = NULL, 
    const uint8_t x = 8, const uint8_t y = 8, const uint8_t w = 64, const uint8_t h = 48)
      : GraphicsItem(parent), _x_label(x_label), _y_label(y_label), GRAPH_X(x), GRAPH_Y(y), GRAPH_W(w), GRAPH_H(h) {
    }
    void addDataPoint(T x, T y) {
        DataPoint<T> temp;
        dataSet.push(DataPoint<T>(x,y),&temp);
        setDirty(true);
    }
    
    DataSet<POINTS, T> getDataSet(){return dataSet;}
    
    void setGraphType(uint8_t type) {
      _graph_type = type;
    }
    void draw(UiContext* context) {
      //confirm font
      context->setCurrentFont(context->getFontSmall());
      //do {
        drawAxes(context);
        if( !dataSet.isEmpty() ) {
          updateXScale(dataSet.xMax() - dataSet.xMin());
          updateYScale(dataSet.yMax() - dataSet.yMin());
          switch(_graph_type) {
            case 0: {
            drawScatter(context);
            break;
            }
            case 1: {
            drawBars(context);
            break;
            }
            case 2: {
            drawLines(context);
            break;
            }
          }
        }
     
        if(_label_last_point) {
            DataPoint<T> p;
            dataSet.first(&p);
            uint8_t mx = scaleX(p.x, _x_scale);
            uint8_t my = scaleY(p.y, _y_scale);
            context->display->setCursor(mx - 24, GRAPH_H - my);
            context->display->print(p.y, DEC);
        }
        //context->display->setCursor( GRAPH_X + GRAPH_W -8, GRAPH_Y + 8 );
        //context->display->print(calcFps(), DEC);

      //} while ( context->display->nextPage() );
      GraphicsItem::draw(context);
    }
};

#endif // End __GRAPH_SCREEN_H__ include guard
