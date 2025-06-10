#ifndef PROCESADOR_VENTAS_H
#define PROCESADOR_VENTAS_H

#include <string>
#include <fstream>
#include <sstream>
#include "Venta.h"
#include "HashMapList.h"
#include "ArbolBinarioAVL.h"
#include "Pila.h"
#include "ColaPrioridad.h"
#include "Grafo.h"

// Clase que gestiona el procesamiento y análisis de ventas
class ProcesadorVentas
{
private:
    // Estructuras de datos principales
    HashMapList<int, Venta> *ventasPorId;   // Acceso rápido por ID_Venta
    ArbolBinarioAVL<Venta> *ventasPorMonto; // Ordenado por monto_total
    Pila<Venta> *pilaEliminaciones;         // Para almacenar ventas eliminadas
    Grafo<std::string> *redLogistica;       // Red de ciudades/envíos

    // Estructuras para resultados precalculados
    HashMapList<std::string, HashMapList<std::string, double> *> *montoPorProductoPais;            // Monto total por producto y país
    HashMapList<std::string, HashMapList<std::string, double> *> *promedioPorCategoriaPais;        // Promedio por categoría y país
    HashMapList<std::string, std::string> *medioEnvioPorPais;                                      // Medio de envío más usado por país
    HashMapList<std::string, std::string> *medioEnvioPorCategoria;                                 // Medio de envío más usado por categoría
    HashMapList<std::string, std::string> *estadoEnvioPorPais;                                     // Estado de envío más frecuente por país
    HashMapList<std::string, ColaPrioridad<std::pair<std::string, double>> *> *topCiudadesPorPais; // Top 5 ciudades por país
    
    std::string diaMayorVentas;                                                                    // Día con mayor monto de ventas
    std::string productoMasVendido;                                                                // Producto más vendido en cantidad
    std::string productoMenosVendido;

    Lista<Venta>* listaVentas;
HashMapList<int, int>* ventasIndicePorID;

    // Método privado para reprocesar resultados tras modificaciones
    void reprocesarResultados();

public:
    // Constructor y destructor
    ProcesadorVentas();
    ~ProcesadorVentas();

    // Carga de datos
    void cargarCSV();

    // Procesamiento de resultados
    void procesarResultados();

    // Modificaciones
    void agregarVenta();
    void eliminarVentaPorID(int id);
    void eliminarVentasPorPaisCiudad(std::string &pais, std::string &ciudad);
    void modificarVenta(int id);

    // Consultas dinámicas
    void listarVentasPorCiudad(const std::string &ciudad);
    void listarVentasPorRangoFechas(const std::string &pais, const std::string &fechaInicio, const std::string &fechaFin);
    void compararPaises(const std::string &pais1, const std::string &pais2);
    void compararProductos(const std::string &producto1, const std::string &producto2);
    void productosPorMontoPromedio(double umbral, bool porEncima);

    // Análisis de red logística
    void analizarRedLogistica();

    // Mostrar resultados principales
    void mostrarResultados();
    void mostrarTop5CiudadesPorPais();
    void mostrarMontoTotalPorProductoPais();
    void mostrarPromedioVentasPorCategoriaPais();
    void mostrarMedioEnvioMasUtilizadoPorPais();
    void mostrarMedioEnvioMasUtilizadoPorCategoria();
    void mostrarDiaMayorVentas();
    void mostrarEstadoEnvioMasFrecuentePorPais();
    void mostrarProductoMasVendido();
    void mostrarProductoMenosVendido();
};

#endif // PROCESADOR_VENTAS_H