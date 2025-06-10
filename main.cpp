#include <iostream>
#include <string>
#include <limits>
#include "ProcesadorVentas.h"

void mostrarMenu()
{
    std::cout << "\n=== Sistema de Analisis de Ventas ===\n";
    std::cout << "1. Mostrar resultados principales\n";
    std::cout << "2. Agregar venta\n";
    std::cout << "3. Eliminar venta por ID\n";
    std::cout << "4. Eliminar ventas por pais/ciudad\n";
    std::cout << "5. Modificar venta\n";
    std::cout << "6. Consultas dinamicas\n";
    std::cout << "7. Analizar red logistica\n";
    std::cout << "8. Salir\n";
    std::cout << "Seleccione una opcion: ";
}

void mostrarSubmenuConsultas()
{
    std::cout << "\n=== Consultas Dinamicas ===\n";
    std::cout << "1. Listar ventas por ciudad\n";
    std::cout << "2. Listar ventas por rango de fechas\n";
    std::cout << "3. Comparar dos paises\n";
    std::cout << "4. Comparar dos productos\n";
    std::cout << "5. Productos con monto promedio por encima de un umbral\n";
    std::cout << "6. Productos con monto promedio por debajo de un umbral\n";
    std::cout << "7. Volver al menu principal\n";
    std::cout << "Seleccione una opcion: ";
}

void submenuConsultasDinamicas(ProcesadorVentas &procesador)
{
    int opcionConsulta;
    do
    {
        mostrarSubmenuConsultas();
        std::cin >> opcionConsulta;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (opcionConsulta)
        {
        case 1:
        {
            std::string ciudad;
            std::cout << "Ingrese la ciudad: ";
            std::getline(std::cin >> std::ws, ciudad); // ✅ Ignora espacios previos automáticamente
            procesador.listarVentasPorCiudad(ciudad);
            break;
        }

        case 2:
        {
            std::string pais, fechaInicio, fechaFin;
            std::cout << "Ingrese el pais: ";
            std::cin.ignore();
            std::getline(std::cin, pais);
            std::cout << "Ingrese fecha inicio (DD/MM/YYYY): ";
            std::getline(std::cin, fechaInicio);
            std::cout << "Ingrese fecha fin (DD/MM/YYYY): ";
            std::getline(std::cin, fechaFin);
            procesador.listarVentasPorRangoFechas(pais, fechaInicio, fechaFin);
            break;
        }
        case 3:
        {
            std::string pais1, pais2;
            std::cout << "Ingrese primer pais: ";
            std::getline(std::cin, pais1);
            std::cout << "Ingrese segundo pais: ";
            std::getline(std::cin, pais2);
            procesador.compararPaises(pais1, pais2);

            break;
        }

        case 4:
        {
            std::string producto1, producto2;
            // 🔥 Limpia bien el buffer
            std::cout << "Ingrese primer producto: ";
            std::getline(std::cin, producto1);
            std::cout << "Ingrese segundo producto: ";
            std::getline(std::cin, producto2);
            procesador.compararProductos(producto1, producto2);
            break;
        }
        case 5:
        {
            double umbral;
            std::cout << "Ingrese umbral: ";
            std::cin >> umbral;
            procesador.productosPorMontoPromedio(umbral, true);
            break;
        }
        case 6:
        {
            double umbral;
            std::cout << "Ingrese umbral: ";
            std::cin >> umbral;
            procesador.productosPorMontoPromedio(umbral, false);
            break;
        }
        case 7:
            std::cout << "Volviendo al menu principal...\n";
            break;
        default:
            std::cout << "Opcion invalida. Intente de nuevo.\n";
        }
    } while (opcionConsulta != 7);
}

int main()
{
    ProcesadorVentas procesador;
    int opcion = 0;
    do
    {
        mostrarMenu();

        std::string entrada;
        std::getline(std::cin, entrada); // Lee la línea completa
        std::stringstream ss(entrada);   // La mete en un stringstream

        if (!(ss >> opcion))
        {
            std::cout << "Opcion invalida. Intente de nuevo.\n";
            continue;
        }

        switch (opcion)
        {
        case 1:
            procesador.mostrarResultados();
            break;
        case 2:
            procesador.agregarVenta();
            break;
        case 3:
        {
            int id;
            std::cout << "Ingrese ID de venta a eliminar: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            procesador.eliminarVentaPorID(id);
            break;
        }

        case 4:
        {
            std::string pais, ciudad;
            std::cout << "Ingrese pais: ";
            std::getline(std::cin >> std::ws, pais); // Limpia espacios en blanco iniciales
            std::cout << "Ingrese ciudad: ";
            std::getline(std::cin >> std::ws, ciudad);
            procesador.eliminarVentasPorPaisCiudad(pais, ciudad);
            break;
        }

        case 5:
        {
            int id;
            std::cout << "Ingrese ID de venta a modificar: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            procesador.modificarVenta(id);
            break;
        }
        case 6:
            submenuConsultasDinamicas(procesador);
            break;
        case 7:
            procesador.analizarRedLogistica();
            break;
        case 8:
            std::cout << "Saliendo del programa...\n";
            break;
        default:
            std::cout << "Opcion invalida. Intente de nuevo.\n";
        }
    } while (opcion != 8);

    return 0;
}
