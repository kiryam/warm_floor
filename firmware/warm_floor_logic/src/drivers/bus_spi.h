#pragma once
#include "stm32f10x.h"

/** @brief  Clear the SPI OVR pending flag.
  * @param  __HANDLE__: specifies the SPI Handle.
  *         This parameter can be SPI where x: 1, 2, or 3 to select the SPI peripheral.
  * @retval None
  */
#define __HAL_SPI_CLEAR_OVRFLAG(__HANDLE__)        \
do{                                                \
    __IO uint32_t tmpreg_ovr = 0x00U;              \
    tmpreg_ovr = (__HANDLE__)->Instance->DR;       \
    tmpreg_ovr = (__HANDLE__)->Instance->SR;       \
  } while(0U)


/**
  * @brief  HAL Lock structures definition
  */
typedef enum
{
  HAL_UNLOCKED = 0x00U,
  HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;

#if (USE_RTOS == 1U)
  /* Reserved for future use */
  #error "USE_RTOS should be 0 in the current HAL release"
#else
  #define __HAL_LOCK(__HANDLE__)                                           \
                                do{                                        \
                                    if((__HANDLE__)->Lock == HAL_LOCKED)   \
                                    {                                      \
                                       return HAL_BUSY;                    \
                                    }                                      \
                                    else                                   \
                                    {                                      \
                                       (__HANDLE__)->Lock = HAL_LOCKED;    \
                                    }                                      \
                                  }while (0U)

  #define __HAL_UNLOCK(__HANDLE__)                                          \
                                  do{                                       \
                                      (__HANDLE__)->Lock = HAL_UNLOCKED;    \
                                    }while (0U)
#endif /* USE_RTOS */

#define __HAL_SPI_GET_FLAG(__HANDLE__, __FLAG__) ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == (__FLAG__))

#define HAL_MAX_DELAY      0xFFFFFFFFU

typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} SPI_StatusTypeDef;

typedef enum
{
  HAL_SPI_STATE_RESET      = 0x00U,    /*!< Peripheral not Initialized                         */
  HAL_SPI_STATE_READY      = 0x01U,    /*!< Peripheral Initialized and ready for use           */
  HAL_SPI_STATE_BUSY       = 0x02U,    /*!< an internal process is ongoing                     */
  HAL_SPI_STATE_BUSY_TX    = 0x03U,    /*!< Data Transmission process is ongoing               */
  HAL_SPI_STATE_BUSY_RX    = 0x04U,    /*!< Data Reception process is ongoing                  */
  HAL_SPI_STATE_BUSY_TX_RX = 0x05U,    /*!< Data Transmission and Reception process is ongoing */
  HAL_SPI_STATE_ERROR      = 0x06U     /*!< SPI error state                                    */
}HAL_SPI_StateTypeDef;


#define HAL_SPI_ERROR_NONE              0x00000000U   /*!< No error             */
#define HAL_SPI_ERROR_MODF              0x00000001U   /*!< MODF error           */
#define HAL_SPI_ERROR_CRC               0x00000002U   /*!< CRC error            */
#define HAL_SPI_ERROR_OVR               0x00000004U   /*!< OVR error            */
#define HAL_SPI_ERROR_FRE               0x00000008U   /*!< FRE error            */
#define HAL_SPI_ERROR_DMA               0x00000010U   /*!< DMA transfer error   */
#define HAL_SPI_ERROR_FLAG              0x00000020U   /*!< Flag: RXNE,TXE, BSY  */

#define SPI_MODE_SLAVE                  0x00000000U
#define SPI_MODE_MASTER                 (SPI_CR1_MSTR | SPI_CR1_SSI)


typedef struct __SPI_HandleTypeDef
{
  SPI_TypeDef                *Instance;    /*!< SPI registers base address */

  SPI_InitTypeDef            Init;         /*!< SPI communication parameters */

  uint8_t                    *pTxBuffPtr;  /*!< Pointer to SPI Tx transfer Buffer */

  uint16_t                   TxXferSize;   /*!< SPI Tx Transfer size */

  __IO uint16_t              TxXferCount;  /*!< SPI Tx Transfer Counter */

  uint8_t                    *pRxBuffPtr;  /*!< Pointer to SPI Rx transfer Buffer */

  uint16_t                   RxXferSize;   /*!< SPI Rx Transfer size */

  __IO uint16_t              RxXferCount;  /*!< SPI Rx Transfer Counter */

  void                       (*RxISR)(struct __SPI_HandleTypeDef * hspi); /*!< function pointer on Rx ISR */

  void                       (*TxISR)(struct __SPI_HandleTypeDef * hspi); /*!< function pointer on Tx ISR */

 //DMA_HandleTypeDef          *hdmatx;      /*!< SPI Tx DMA Handle parameters   */

 // DMA_HandleTypeDef          *hdmarx;      /*!< SPI Rx DMA Handle parameters   */

 // HAL_LockTypeDef            Lock;         /*!< Locking object                 */

  __IO HAL_SPI_StateTypeDef  State;        /*!< SPI communication state */

  __IO uint32_t              ErrorCode;    /*!< SPI Error code */

  HAL_LockTypeDef            Lock;

}SPI_HandleTypeDef;


SPI_StatusTypeDef SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);
