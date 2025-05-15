#ifndef KAMIKAZE_CORE_RTOS_HPP_
#define KAMIKAZE_CORE_RTOS_HPP_

#include "../../include/common.hpp"

namespace Kamikaze {
namespace Core {

// Forward declarations
class Task;

/**
 * @brief Prioritas task RTOS
 */
enum class TaskPriority : Uint8 {
    LOW = 0U,
    NORMAL = 1U,
    HIGH = 2U,
    CRITICAL = 3U
};

/**
 * @brief Status task RTOS
 */
enum class TaskStatus : Uint8 {
    READY = 0U,
    RUNNING = 1U,
    BLOCKED = 2U,
    SUSPENDED = 3U,
    DELETED = 4U
};

/**
 * @brief Interface untuk task RTOS
 */
class Task {
public:
    Task();
    virtual ~Task();

    /**
     * @brief Method yang dijalankan oleh task
     * @return Status code setelah task selesai
     */
    virtual Uint32 run() = 0;
    
    /**
     * @brief Mendapatkan status task
     * @return Status task saat ini
     */
    TaskStatus getStatus() const;
    
    /**
     * @brief Mendapatkan prioritas task
     * @return Prioritas task
     */
    TaskPriority getPriority() const;
    
    /**
     * @brief Mendapatkan ID task
     * @return ID task
     */
    Uint32 getId() const;

protected:
    TaskStatus status_;
    TaskPriority priority_;
    Uint32 id_;
    
private:
    // Mencegah copy constructor dan assignment operator
    Task(const Task&);
    Task& operator=(const Task&);
};

/**
 * @brief Kelas untuk RTOS
 */
class Rtos {
public:
    /**
     * @brief Mendapatkan instance singleton RTOS
     * @return Referensi ke instance RTOS
     */
    static Rtos& getInstance();
    
    /**
     * @brief Inisialisasi RTOS
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Memulai RTOS scheduler
     * @return true jika berhasil, false jika gagal
     */
    bool start();
    
    /**
     * @brief Mendaftarkan task
     * @param task Task untuk didaftarkan
     * @param priority Prioritas task
     * @return ID task jika berhasil, 0 jika gagal
     */
    Uint32 registerTask(Task* task, TaskPriority priority);
    
    /**
     * @brief Menghapus task
     * @param taskId ID task yang akan dihapus
     * @return true jika berhasil, false jika gagal
     */
    bool deleteTask(Uint32 taskId);
    
    /**
     * @brief Menunda task untuk waktu tertentu
     * @param milliseconds Waktu delay dalam milidetik
     */
    static void delay(Uint32 milliseconds);
    
    /**
     * @brief Mendapatkan tick count RTOS
     * @return Tick count saat ini
     */
    static Uint32 getTicks();

private:
    // Private constructor untuk singleton
    Rtos();
    
    // Mencegah copy constructor dan assignment operator
    Rtos(const Rtos&);
    Rtos& operator=(const Rtos&);
    
    // Implementasi internal
    bool initialized_;
    
    // Konstanta
    static const Uint8 MAX_TASKS = 16U;
};

} // namespace Core
} // namespace Kamikaze

#endif // KAMIKAZE_CORE_RTOS_HPP_ 