#ifndef ANIMATION_H
#define ANIMATION_H

#include <QMap>
#include <QEasingCurve>
#include <QColor>

namespace panda
{

template<class T> T lerp(const T& v1, const T& v2, const double& amt);

template<>
QColor lerp(const QColor& v1, const QColor& v2, const double& amt);

template<class T> T lerp(const T& v1, const T& v2, const double& amt)
{
    return v1 * (1.0-amt) + v2 * amt;
}

template <class T>
class Animation
{
public:
    Animation() {}
    virtual ~Animation() {}

    T get(const double& at) const;
    T& getValue(const double& key) { return map[key]; }
    T& getValueAtIndex(int index);
    T getValueAtIndexConst(int index) const;

    int size() const { return map.size(); }
    void clear() { map.clear(); }
    void add(const double& key, const T& value) { map.insert(key, value); }
    void move(const double& keyFrom, const double& keyTo);
    void remove(const double& key) { map.remove(key); }

    virtual QList<double> getKeys() const { return map.uniqueKeys(); }
    QList<T> getValues() const { return map.values(); }

    typedef QMapIterator<double, T> Iterator;
    Iterator getIterator() const { return Iterator(map); }

    QEasingCurve interpolation;

protected:
    QMap<double, T> map;
};

typedef Animation<QColor> Gradient;

} // namespace panda

#endif // ANIMATION_H
