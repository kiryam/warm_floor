#include "platform.h"
#include "bus_spi.h"
#include "stdint.h"
#include "stddef.h"

/**
  * @brief Handle SPI Communication Timeout.
  * @param hspi: pointer to a SPI_HandleTypeDef structure that contains
  *              the configuration information for SPI module.
  * @param Flag: SPI flag to check
  * @param State: flag state to check
  * @param Timeout: Timeout duration
  * @param Tickstart: tick start value
  * @retval HAL status
  */
static SPI_StatusTypeDef SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, uint32_t State, uint32_t Timeout, uint32_t Tickstart)
{
  while((((hspi->Instance->SR & Flag) == (Flag)) ? SET : RESET) != State)
  {
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U) || ((GetTick()-Tickstart) >= Timeout))
      {
        /* Disable the SPI and reset the CRC: the CRC value should be cleared
        on both master and slave sides in order to resynchronize the master
        and slave for their respective CRC calculation */

        /* Disable TXE, RXNE and ERR interrupts for the interrupt process */
        //__HAL_SPI_DISABLE_IT(hspi, (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR));

        //if((hspi->Init.SPI_Mode == SPI_MODE_MASTER)&&((hspi->Init.Direction == SPI_DIRECTION_1LINE)||(hspi->Init.Direction == SPI_DIRECTION_2LINES_RXONLY)))
        //{
          /* Disable SPI peripheral */
       //   __HAL_SPI_DISABLE(hspi);
       // }

        /* Reset CRC Calculation */
        //if(hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
        //{
        //  SPI_RESET_CRC(hspi);
       // }

        hspi->State= HAL_SPI_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hspi);

        return HAL_TIMEOUT;
      }
    }
  }

  return HAL_OK;
}


/**
  * @brief Handle to check BSY flag before start a new transaction.
  * @param hspi: pointer to a SPI_HandleTypeDef structure that contains
  *              the configuration information for SPI module.
  * @param Timeout: Timeout duration
  * @param Tickstart: tick start value
  * @retval HAL status
  */
static SPI_StatusTypeDef SPI_CheckFlag_BSY(SPI_HandleTypeDef *hspi, uint32_t Timeout, uint32_t Tickstart)
{
  /* Control the BSY flag */
  if(SPI_WaitFlagStateUntilTimeout(hspi, SPI_I2S_FLAG_BSY, RESET, Timeout, Tickstart) != HAL_OK)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
    return HAL_TIMEOUT;
  }
  return HAL_OK;
}

SPI_StatusTypeDef SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout) {
  uint32_t tmp = 0U, tmp1 = 0U;
#if (USE_SPI_CRC != 0U)
  __IO uint16_t tmpreg1 = 0U;
#endif /* USE_SPI_CRC */
  uint32_t tickstart = 0U;
  /* Variable used to alternate Rx and Tx during transfer */
  uint32_t txallowed = 1U;
  SPI_StatusTypeDef errorcode = HAL_OK;

  /* Check Direction parameter */
  assert_param(IS_SPI_DIRECTION_MODE(hspi->Init.SPI_Direction));

  /* Process Locked */
  __HAL_LOCK(hspi);

  /* Init tickstart for timeout management*/
  tickstart = GetTick();

  tmp  = hspi->State;
  tmp1 = hspi->Init.SPI_Mode;

  if(!((tmp == HAL_SPI_STATE_READY) || ((tmp1 == SPI_MODE_MASTER) && (hspi->Init.SPI_Direction == SPI_Direction_2Lines_FullDuplex) && (tmp == HAL_SPI_STATE_BUSY_RX)))) {
    errorcode = HAL_BUSY;
    goto error;
  }

  if((pTxData == NULL) || (pRxData == NULL) || (Size == 0U)) {
    errorcode = HAL_ERROR;
    goto error;
  }

  /* Don't overwrite in case of HAL_SPI_STATE_BUSY_RX */
  if(hspi->State == HAL_SPI_STATE_READY) {
    hspi->State = HAL_SPI_STATE_BUSY_TX_RX;
  }

  /* Set the transaction information */
  hspi->ErrorCode   = HAL_SPI_ERROR_NONE;
  hspi->pRxBuffPtr  = (uint8_t *)pRxData;
  hspi->RxXferCount = Size;
  hspi->RxXferSize  = Size;
  hspi->pTxBuffPtr  = (uint8_t *)pTxData;
  hspi->TxXferCount = Size;
  hspi->TxXferSize  = Size;

  /*Init field not used in handle to zero */
  hspi->RxISR       = NULL;
  hspi->TxISR       = NULL;

#if (USE_SPI_CRC != 0U)
  /* Reset CRC Calculation */
  if(hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
  {
    SPI_RESET_CRC(hspi);
  }
#endif /* USE_SPI_CRC */

  /* Check if the SPI is already enabled */
 // if((hspi->Instance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE) {
    /* Enable SPI peripheral */
  //  __HAL_SPI_ENABLE(hspi);
 // }

  /* Transmit and Receive data in 16 Bit mode */
  if(hspi->Init.SPI_DataSize == SPI_DataSize_16b) {
    if((hspi->Init.SPI_Mode == SPI_MODE_SLAVE) || (hspi->TxXferCount == 0x01U)) {
      hspi->Instance->DR = *((uint16_t *)pTxData);
      pTxData += sizeof(uint16_t);
      hspi->TxXferCount--;
    }
    while ((hspi->TxXferCount > 0U) || (hspi->RxXferCount > 0U)) {
      /* Check TXE flag */
      if(txallowed && (hspi->TxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_I2S_FLAG_TXE))) {
        hspi->Instance->DR = *((uint16_t *)pTxData);
        pTxData += sizeof(uint16_t);
        hspi->TxXferCount--;
        /* Next Data is a reception (Rx). Tx not allowed */
        txallowed = 0U;

#if (USE_SPI_CRC != 0U)
        /* Enable CRC Transmission */
        if((hspi->TxXferCount == 0U) && (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE))
        {
          SET_BIT(hspi->Instance->CR1, SPI_CR1_CRCNEXT);
        }
#endif /* USE_SPI_CRC */
      }

      /* Check RXNE flag */
      if((hspi->RxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_I2S_FLAG_RXNE))) {
    	  *((uint16_t *)pRxData) = hspi->Instance->DR;
    	  pRxData += sizeof(uint16_t);
    	  hspi->RxXferCount--;
    	  /* Next Data is a Transmission (Tx). Tx is allowed */
    	  txallowed = 1U;
      }
      if((Timeout != HAL_MAX_DELAY) && ((GetTick()-tickstart) >=  Timeout)) {
    	  errorcode = HAL_TIMEOUT;
    	  goto error;
      }
    }
  } else { /* Transmit and Receive data in 8 Bit mode */
    if((hspi->Init.SPI_Mode == SPI_MODE_SLAVE) || (hspi->TxXferCount == 0x01U)) {
      *((__IO uint8_t*)&hspi->Instance->DR) = (*pTxData);
      pTxData += sizeof(uint8_t);
      hspi->TxXferCount--;
    }
    while((hspi->TxXferCount > 0U) || (hspi->RxXferCount > 0U)) {
      /* check TXE flag */
      if(txallowed && (hspi->TxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_I2S_FLAG_TXE))) {
        *(__IO uint8_t *)&hspi->Instance->DR = (*pTxData++);
        hspi->TxXferCount--;
        /* Next Data is a reception (Rx). Tx not allowed */
        txallowed = 0U;

#if (USE_SPI_CRC != 0U)
        /* Enable CRC Transmission */
        if((hspi->TxXferCount == 0U) && (hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE))
        {
          SET_BIT(hspi->Instance->CR1, SPI_CR1_CRCNEXT);
        }
#endif /* USE_SPI_CRC */
      }

      /* Wait until RXNE flag is reset */
      if((hspi->RxXferCount > 0U) && (__HAL_SPI_GET_FLAG(hspi, SPI_I2S_FLAG_RXNE))) {
        (*(uint8_t *)pRxData++) = hspi->Instance->DR;
        hspi->RxXferCount--;
        /* Next Data is a Transmission (Tx). Tx is allowed */
        txallowed = 1U;
      }
      if((Timeout != HAL_MAX_DELAY) && ((GetTick()-tickstart) >=  Timeout)) {
        errorcode = HAL_TIMEOUT;
        goto error;
      }
    }
  }

#if (USE_SPI_CRC != 0U)
  /* Read CRC from DR to close CRC calculation process */
  if(hspi->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)
  {
    /* Wait until TXE flag */
    if(SPI_WaitFlagStateUntilTimeout(hspi, SPI_FLAG_RXNE, SET, Timeout, tickstart) != HAL_OK)
    {
      /* Error on the CRC reception */
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_CRC);
      errorcode = HAL_TIMEOUT;
      goto error;
    }
    /* Read CRC */
    tmpreg1 = hspi->Instance->DR;
    /* To avoid GCC warning */
    UNUSED(tmpreg1);
  }

  /* Check if CRC error occurred */
  if(__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_CRCERR) != RESET)
  {
    /* Check if CRC error is valid or not (workaround to be applied or not) */
    if (SPI_ISCRCErrorValid(hspi) == SPI_VALID_CRC_ERROR)
    {
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_CRC);

      /* Reset CRC Calculation */
      SPI_RESET_CRC(hspi);

   	  errorcode = HAL_ERROR;
    }
    else
    {
      __HAL_SPI_CLEAR_CRCERRFLAG(hspi);
    }
  }
#endif /* USE_SPI_CRC */

  /* Wait until TXE flag */
  if(SPI_WaitFlagStateUntilTimeout(hspi, SPI_I2S_FLAG_TXE, SET, Timeout, tickstart) != HAL_OK) {
    errorcode = HAL_TIMEOUT;
    goto error;
  }

  /* Check Busy flag */
  if(SPI_CheckFlag_BSY(hspi, Timeout, tickstart) != HAL_OK) {
    errorcode = HAL_ERROR;
    hspi->ErrorCode = HAL_SPI_ERROR_FLAG;
    goto error;
  }

  /* Clear overrun flag in 2 Lines communication mode because received is not read */
  if(hspi->Init.SPI_Direction == SPI_Direction_2Lines_FullDuplex) {
    __HAL_SPI_CLEAR_OVRFLAG(hspi);
  }

error :
  hspi->State = HAL_SPI_STATE_READY;
  __HAL_UNLOCK(hspi);
  return errorcode;
}
