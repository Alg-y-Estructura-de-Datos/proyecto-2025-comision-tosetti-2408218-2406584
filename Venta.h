#ifndef VENTA_H
#define VENTA_H

#include <string>
#include <chrono>
#include <iostream>

// Clase que representa un registro de venta del archivo CSV
class Venta {
public:
    int id_venta;
    std::string fecha; // Formato: AAAA-MM-DD
    std::string pais;
    std::string ciudad;
    std::string cliente;
    std::string producto;
    std::string categoria;
    int cantidad;
    double precio_unitario;
    double monto_total;
    std::string medio_envio;
    std::string estado_envio;

    // Constructor por defecto
    Venta() : id_venta(0), fecha(""), pais(""), ciudad(""), cliente(""), 
              producto(""), categoria(""), cantidad(0), precio_unitario(0.0), 
              monto_total(0.0), medio_envio(""), estado_envio("") {}

    // Constructor con parámetros
    Venta(int id, std::string f, std::string p, std::string c, std::string cl, 
          std::string prod, std::string cat, int cant, double precio, double monto, 
          std::string envio, std::string estado)
        : id_venta(id), fecha(f), pais(p), ciudad(c), cliente(cl), 
          producto(prod), categoria(cat), cantidad(cant), precio_unitario(precio), 
          monto_total(monto), medio_envio(envio), estado_envio(estado) {}

    // Operador de igualdad para comparar ventas (usado en HashMapList y búsquedas)
    bool operator==(const Venta& other) const {
        return id_venta == other.id_venta;
    }

    // Operador menor que para ArbolBinarioAVL (ordenado por monto_total)
    bool operator<(const Venta& other) const {
        return monto_total < other.monto_total;
    }

    // Operador mayor que para ArbolBinarioAVL
    bool operator>(const Venta& other) const {
        return monto_total > other.monto_total;
    }

    // Método para imprimir la venta (útil para consultas)
    void imprimir() const {
        std::cout << "ID: " << id_venta << ", Fecha: " << fecha 
                  << ", País: " << pais << ", Ciudad: " << ciudad 
                  << ", Cliente: " << cliente << ", Producto: " << producto 
                  << ", Categoría: " << categoria << ", Cantidad: " << cantidad 
                  << ", Precio Unitario: $" << precio_unitario 
                  << ", Monto Total: $" << monto_total 
                  << ", Medio de Envío: " << medio_envio 
                  << ", Estado: " << estado_envio << std::endl;
    }
};

// Estructura auxiliar para almacenar resultados de top 5 (ciudades, productos, etc.)
struct ResultadoTop {
    std::string nombre; // Ejemplo: nombre de ciudad o producto
    double valor;      // Ejemplo: monto total o cantidad
    ResultadoTop(std::string n, double v) : nombre(n), valor(v) {}
};

// Clase para medir eficiencia (tiempo y condicionales)
class Metricas {
private:
    std::string nombre_proceso; // Nombre del proceso medido
    std::string estructura;     // Estructura o algoritmo usado
    int condicionales;          // Contador de condicionales (if)
    std::chrono::duration<double, std::milli> tiempo; // Tiempo en milisegundos
    std::chrono::time_point<std::chrono::high_resolution_clock> inicio;

public:
    // Constructor
    Metricas(std::string nombre, std::string estr) 
        : nombre_proceso(nombre), estructura(estr), condicionales(0), tiempo(0) {
        iniciar();
    }

    // Iniciar medición
    void iniciar() {
        inicio = std::chrono::high_resolution_clock::now();
    }

    // Finalizar medición
    void finalizar() {
        auto fin = std::chrono::high_resolution_clock::now();
        tiempo = fin - inicio;
    }

    // Incrementar contador de condicionales
    void incrementarCondicional() {
        condicionales++;
    }

    // Mostrar resultados
    void mostrar() const {
        std::cout << "Proceso: " << nombre_proceso << std::endl;
        std::cout << "Estructura/Algoritmo: " << estructura << std::endl;
        std::cout << "Tiempo de ejecución: " << tiempo.count() << " ms" << std::endl;
        std::cout << "Condicionales utilizados: " << condicionales << std::endl;
        std::cout << "------------------------" << std::endl;
    }

    

};



#endif // VENTA_H