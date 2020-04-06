#ifndef __GraphicsItem_H__
#define __GraphicsItem_H__

#include "ui_context.h"
//#include "menu_bar.h"
//typedef void(*Callback) ();
typedef void(*func_ptr) (void* sender);

enum TextAlign : uint8_t {
  NoAlign = 0,
  AlignLeft = 1, 
  AlignRight = 2, 
  AlignHCenter = 4, 
  AlignTop = 8, 
  AlignBottom = 16,
  AlignVCenter = 32
};

//#define MAX_CHILDREN 8

/*!
 * @brief the GraphicsItem class is the base class for graphics
 * that are shown on a screen.
 * 
 */
class GraphicsItem {
  GraphicsItem* _parent = nullptr;
  GraphicsItem* _child = nullptr;
  // Graphics items default to dirty when constructed
  bool _isDirty = true; 
  uint8_t _x ,_y, _w, _h = 0;
  // Graphics items default to visible
  bool _isVisible = true;

  public:
  void setDirty(bool dirty) {
    if(_parent) {
      _parent->setDirty(dirty);
    }
    else {
      _isDirty = dirty;
    }
  }
  void setVisible(bool isVisible){
    _isVisible = isVisible;
  }
  bool isVisible() {return _isVisible;}

  bool isDirty(){ return _isDirty; }

  void setPos(uint8_t x, uint8_t y) {
    _x = x;
    _y = y;
    setDirty(true);
  }

  void setSize(uint8_t w, uint8_t h) {
    _w = w;
    _h = h;
    setDirty(true);
  }

  GraphicsItem* parent() {return _parent;}

  void addChild(GraphicsItem* item) {
    if(_child == nullptr) {
      _child = item;
    }
    else {
      _child->addChild(item);
    }
    /*
    for(uint8_t i = 0; i < MAX_CHILDREN; ++i) {
      if(_children[i] == NULL) {
      _children[i] = item;
      return 0;
      }
    }
    return -1;*/
  }

  GraphicsItem* removeChild(GraphicsItem* item) {
    if(item == _child) {
      // remove just this child
      GraphicsItem* temp = _child;
      _child = nullptr;
      return temp; // returns the child that was removed
    }
    if(_child != nullptr) {
      // try to remove the child from another child
      return _child->removeChild(item);
    }
    else {
      return nullptr;
    }
  /*
    if(_child->)
    for(uint8_t i = 0; i < MAX_CHILDREN; ++i) {
      if(_children[i] == item) {
      GraphicsItem* temp = _children[i];
      _children[i] = NULL;
      return temp;
      }
    }
    return NULL;*/
  }

  virtual void draw(UiContext* context) {
    //if( !_isVisible ) return; // uncomment for parent to hide children

    if(_child != nullptr) {
      _child->draw(context);
    }
    /*
    uint8_t i = MAX_CHILDREN;
    do {
      if( _children[i - 1] != NULL ) {
        _children[i - 1]->draw(context);
      }
    } while ( --i );
    /*
    for(uint8_t i = 0; i < MAX_CHILDREN; ++i) {
      if( _children[i] != NULL ) {
        //if( _children[i]->isDirty() ){
          _children[i]->draw(context);
        //}
      }
    }*/
  }

  uint8_t left() {
    return _x;
  }
  uint8_t right() {
    return _x + _w;
  }
  uint8_t top() {
    return _y;
  }
  uint8_t bottom() {
    return _y + _h;
  }

  uint8_t width() {return _w;}
  uint8_t height() {return _h;}

  /*!
   * @brief Construct a new Graphics Item object
   * 
   * @param parent 
   * @param x 
   * @param y 
   * @param w 
   * @param h 
   */
  GraphicsItem(GraphicsItem* parent = nullptr, uint8_t x = 0, uint8_t y = 0, uint8_t w = 0, uint8_t h = 0) 
  : _parent(parent), _x(x), _y(y), _w(w), _h(h) {}

}; // GraphicsItem class

/*!
 * @brief the TextItem class provides a block of text that
 * can be displayed on a screen
 * 
 * @tparam MAX_CHAR the maxumum number of characters this text item can store
 * this is used to reserve an array to hold the text characters
 */
template <uint8_t MAX_CHAR>
class TextItem : public GraphicsItem {
  TextAlign _alignment = 0;
  char _text[MAX_CHAR + 1];
  const uint8_t* _font;

  uint8_t xPadding(TextAlign alignment, uint8_t text_width) {
    /*Serial.print("alignment: ");
    Serial.print(alignment, DEC);
    Serial.print( " bin: ");
    Serial.println(alignment, BIN);*/
    switch(alignment & B111) {
    case AlignLeft: {
      return parent()->left() + left();
    }
    case AlignRight: {
      return ( parent()->right() - text_width);
    }
    case AlignHCenter: {
      return (parent()->width() - text_width) >> 1;
    }
    default:
      return left();
    }
  }

  uint8_t yPadding(TextAlign alignment, uint8_t text_ascent) {
    switch(alignment & B111000) {
    case AlignTop: {
      return parent()->top() + text_ascent + top();
    }
    case AlignBottom: {
      return parent()->height();
    }
    case AlignVCenter: {
      return ( (parent()->height() - text_ascent) >> 1) + text_ascent;
    }
    default:
      return top();
    }
  }

  public:
  TextItem(GraphicsItem* parent, const uint8_t* font, char* text, uint8_t alignment = NoAlign) 
  : GraphicsItem(parent), _font(font), _alignment((TextAlign)alignment){
  setText(text);
  }

  void draw(UiContext* context){
    if(this->isVisible()){
      //Serial.print("textitem::draw ");
      //Serial.println(_text);
      context->setCurrentFont(_font);
      uint8_t xPad = xPadding(_alignment, context->display->getStrWidth(_text));
      uint8_t yPad = yPadding(_alignment, context->display->getAscent());
      context->display->setCursor(xPad, yPad);
      context->display->print(_text);
      GraphicsItem::draw(context); // draw children
    }
  }

  void setText(char* text) {
    strncpy(_text, text, MAX_CHAR);
    setDirty(true);
  }

  char* text() {return _text;}
}; //TextItem class definition


/*!
 * @brief The LineItem class provides a line that
 * can be added to a screen
 * 
 */
class LineItem : public GraphicsItem {

uint8_t _x2;
uint8_t _y2;

public:
LineItem(GraphicsItem* parent, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) 
: GraphicsItem(parent), _x2(x2), _y2(y2){
	setPos(x1,y1);
	}
  void draw(UiContext* context){
  context->display->drawLine(left(),top(),_x2,_y2);
  //Serial.println("draw line");
  GraphicsItem::draw(context);
}

/*void setPos(uint8_t_ x, uint8_t y) {
  
}*/

void move(int8_t dx, int8_t dy) {
  _x2 += dx;
  _y2 += dy;
  uint8_t last_x = left();
  uint8_t last_y = top();
  setPos(last_x + dx, last_y + dy);
}

void setLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
   _x2 = x2;
   _y2 = y2;
  
  setPos(x1, y1);
}
}; //LineItem class definition

class MenuBar : public GraphicsItem {
public:
  MenuBar(GraphicsItem* parent, uint8_t x, uint8_t y, uint8_t w, uint8_t h) : GraphicsItem(parent, x, y, w, h){}

  void draw(UiContext* context) {
    //Serial.println("bar draw");
    context->display->drawLine(left(), height(), width(), height());
    GraphicsItem::draw(context);
  }
}; //MenuBar class definition

/*!
 * @brief the NumberItem class provides a number that can
 * be displayed on a screen. It looks like a text item, but
 * incrementing or decrementing the value works like a base-10
 * number.
 * 
 * @tparam T the type of number (uint_8_t, int8_t, float)
 */
template <typename T>
class NumberItem : public GraphicsItem {
  volatile T _value = 0;
  uint8_t _precision = 2;
  T _min_value;
  T _max_value;
  const uint8_t * _font;
  Callback _value_changed_callback = nullptr;

  // Delegate printing the value to specialized template functions
  template<typename TI>
  struct identity { typedef T type; };

  template <typename TL>
  void printValue(UiContext* context, identity<TL>) {
    //context->display->setDrawColor(2);
    context->display->setCursor( left(), bottom() );
    context->display->print(_value, DEC);
  }
  void printValue(UiContext* context, identity<float>) {
    context->display->setCursor( left(), bottom() );
    context->display->print(_value, _precision);
  }  
      
  public:
  NumberItem(GraphicsItem* parent, const uint8_t* font, 
  uint8_t x = 0, uint8_t y = 0, uint8_t w = 0, uint8_t h = 0, 
  T min_value = 0, T max_value = 100) 
  : GraphicsItem(parent, x, y, w, h), 
  _font(font), _min_value(min_value), _max_value(max_value){}
  
  /**
   * @brief adds one to the current value. This function
   * does no bounds checking
   */
  void increment() {
    setValue(_value + 1);
  }
  /**
   * @brief subtracts one from the current value. This function
   * does no bounds checking
   */
  void decrement() {
    setValue(_value - 1);
  }

  /**
   * @brief Set the value variable
   * 
   * @param value the new value
   * @param respond if true, attempts to call the value changed callback 
   * (default = true). Set this to false to temporarily ignore changes
   */
  void setValue(const T value, const bool respond = true) {
    _value = value;
    if (_value > _max_value) {
      _value = _max_value;
    }
    if (_value < _min_value) {
      _value = _min_value;
    }
    if ( _value_changed_callback != nullptr && respond ) {
      (*_value_changed_callback)();
    }
    //setSize(); // adjust width
    setDirty(true);
  }
  const T value() {
    return _value;
  }
  const T minValue() {
    return _min_value;
  }
  const T maxValue() {
    return _max_value;
  }

  void setPrecision(const uint8_t decimals) {
    _precision = decimals;
  }
  /**
   * @brief Set the Value Changed Callback variable
   * this callback is called when the current value is changed
   * @param c 
   */
  void setValueChangedCallback(Callback c) {
    _value_changed_callback = c;
  }
  // printing is delegated to private, specialized template functions
      // to handle special cases like floating point numbers
      template <typename TL>
      void printValue(UiContext* context) {
        printValue(context, identity<TL>());
      }
      
      void draw(UiContext* context) {
          //print value
          context->setCurrentFont(_font);
          printValue<T>(context);
          GraphicsItem::draw(context);
      }
}; //NumberItem class definition


class ProgressBar : public GraphicsItem {
  public:
    ProgressBar(GraphicsItem* parent, uint8_t x, uint8_t y, uint8_t w, uint8_t h) : GraphicsItem(parent, x, y, w, h){}

  void draw(UiContext* context) {
    //Serial.println("bar draw");
    context->display->drawLine(left(), height(), width(), height());
    GraphicsItem::draw(context);
  }
}; // ProgressBar class definition

#endif // End __GraphicsItem_H__ include guard
