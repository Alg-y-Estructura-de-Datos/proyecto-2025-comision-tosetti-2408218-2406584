// HashEntry.h (updated)
#ifndef U05_HASH_HASHMAP_HASHENTRY_H_
#define U05_HASH_HASHMAP_HASHENTRY_H_

template <class K, class T>
class HashEntry
{
private:
    K clave;
    T valor;

public:
    HashEntry() : clave(K()), valor(T()) {}

    HashEntry(K c, T v)
    {
        clave = c;
        valor = v;
    }

    K getClave() const
    {
        return clave;
    }

    void setClave(K c)
    {
        clave = c;
    }

    T &getValor()
    {
        return valor;
    }

    void setValor(T v)
    {
        valor = v;
    }

    // Added operator== to compare entries by key
    bool operator==(const HashEntry<K, T> &other) const
    {
        return clave == other.clave;
    }
};

#endif // U05_HASH_HASHMAP_HASHENTRY_H_