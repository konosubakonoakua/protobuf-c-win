/// includes
#include "sensor.pb-c.h"
#include "protobuf-c.h"
#include "stdio.h"
/// prototypes
static void ncopy(void* dst, void* src, uint32_t nbytes);
static void ncopy8to32(uint32_t *dst, uint8_t *src, uint32_t nbytes);

/// types
typedef struct sensor_reg
{
    uint8_t id;
    uint8_t sn[5];
    uint8_t is_registed;
}sensor_reg_t;

typedef struct sensor_data
{
    int32_t wave32[60];
    uint16_t vol_dc_offset;
    uint16_t cur_rms_100x;
    uint16_t cur_rms_raw;
    uint16_t vol_battery;
    uint16_t temperature;
    // uint8_t wave[4][30];
    char sn[5];
}sensor_data_t;

typedef union nrf_package_1
{
    struct {
        uint8_t size;   /* package size, typically 0x1f */
        uint8_t bgn;    /* typically, 0xff */
        uint8_t id[5];  /* serial number array */
        uint16_t vol_dc_offset; /* DC offset voltage, x100 */
        uint16_t cur_rms_100x;  /* RMS Current x100 */
        uint16_t cur_rms_raw;   /* origin RMS Current value */
        uint16_t vol_battery;   /* voltage of inner battery, x100 */
        uint16_t temperature;   /* temperature in degrees, x100 */
    }t;
    uint8_t buf[32];
}nrf_pkg_1_t;
typedef union nrf_package_2
{
    struct {
        uint8_t size;       /* package size, typically 0x1f */
        uint8_t bgn;        /* typically, id[4] */
        uint16_t wave[15];  /* serial number array */
    }t;
    uint8_t buf[32];
}nrf_pkg_2_t;


/// variables
uint8_t nrfbuf1[32] = {
    0x1f,
    0xFF, 0x61, 0x6B, 0x75, 0x61, 0x30, 0x00, 0x10, 0x27, 0x10, 0x27, 0x10, 0x27, 0x58,
    0x02, 0xC4, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
/* sensor */
nrf_pkg_1_t pkg_1;
nrf_pkg_2_t pkg_2[4];
int32_t current_transient[60];
uint32_t msg_len;
uint8_t message_pack_buffer[512];
Sensor__Message sensor_msg = SENSOR__MESSAGE__INIT;
Sensor__Message *msg = NULL, *msg2 = NULL;
#ifdef __CUR_HAS_3_PHASE__
Sensor__ThreePhase three_phase = SENSOR__THREE_PHASE__INIT;
  Sensor__SinglePhase single_phase[3] = {SENSOR__SINGLE_PHASE__INIT};
  /* pointers point to elements in single_phase[] */
  Sensor__SinglePhase* single_pahse_p[3] = {
    &single_phase[0], &single_phase[1], &single_phase[2]
  };
#else
Sensor__SinglePhase single_phase = SENSOR__SINGLE_PHASE__INIT;
#endif
Sensor__DeviceRegister dev_register = SENSOR__DEVICE_REGISTER__INIT;
/* flags */
uint8_t is_receiving_waveform;
uint8_t cnt_waveform_frame;

/// functions
static void ncopy(void* dst, void* src, uint32_t nbytes)
{
    for(uint8_t i = 0; i < nbytes; i++)
    {
        ((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
    }
}
static void ncopy8to32(uint32_t *dst, uint8_t *src, uint32_t nbytes)
{
    while(nbytes--)
    {
        *dst++ = *src++;
    }
}
static void init_single_phase(Sensor__SinglePhase* p)
{
    p->message_type = 1;
    p->phase = 1;
    p->instantaneouscurrent = current_transient;
    p->n_instantaneouscurrent = 60;
}
static void init_dev_resgister(Sensor__DeviceRegister *p)
{
    p->sn = "akua0";
    p->device_type = 123;
}
#ifdef __CUR_HAS_3_PHASE__
static void init_three_phase(Sensor__ThreePhase* p)
  {
    p->n_singlephases = 3;
    p->singlephases = single_pahse_p;
  }
#endif
static void init_sensor_msg(Sensor__Message* p, uint8_t type)
{
    p->message_type = 1;
    switch(type)
    {
        /* register type */
        case 1:
            p->message_type = 1;
            p->register_ = &dev_register;
            p->singlephase = NULL;
#ifdef __CUR_HAS_3_PHASE__
            p->threephase = NULL;
#endif
            return;
            /* singlephase type */
        case 2:
            p->message_type = 2;
            p->register_ = NULL;
            p->singlephase = &single_phase;
#ifdef __CUR_HAS_3_PHASE__
            p->threephase = NULL;
#endif
            return;
#ifdef __CUR_HAS_3_PHASE__
        case 3:
        p->message_type = 3;
        p->register_ = NULL;
        p->singlephase = NULL;
        p->threephase = &three_phase;
        return;
#endif
        default:
            return;
    }
    // p->singlephase = &single_phase;



    // p->register_ = NULL;
}


int main(void)
{
    /* macros */
    #define T1          pkg_1.t
    #define T2          pkg_2.t
    #define buf1        pkg_1.buf
    #define buf2(i)     pkg_2[i].buf
    #define wav(i)      pkg_2[i].t.wave
    #define wav32_tab   current_transient
    #define sn_tab      pkg_1.t.id
    #define flg_set(v)  (v) = 1
    #define flg_clr(v)  (v) = 0
    #define __nrf_conn_check__ /* check communication with nrf24l01 good or not. */
    #define __nrf_debug__ /* print debug info using nrf24l01. */

    /* variables */
    char sn_str[6] = {'0', '0', '0', '0', '0', 0}; /* 0 for '\0' */
    /* protobuf initialization */
    init_single_phase(&single_phase);
    init_dev_resgister(&dev_register);


    /* read data received by nrf24 */
    /* copy whole array */
    ncopy(buf1, nrfbuf1, 32);
    for(uint8_t i = 0; i < 60; i++)
        current_transient[i] = i + '0';
    /* protobuf serialization */
    /* send register package first */
    ncopy(sn_str, sn_tab, 5);
    dev_register.sn = sn_str;
    init_sensor_msg(&sensor_msg, 1);
    /* send or save to sdcard */
    msg_len =sensor__message__get_packed_size(&sensor_msg);
    sensor__message__pack(&sensor_msg, message_pack_buffer);
    printf(message_pack_buffer, msg_len);
    msg = sensor__message__unpack(NULL, msg_len, message_pack_buffer);
    printf("\r\nsn: ");
    if(msg!=NULL)
    {
        printf(((Sensor__Message*)msg)->register_->sn);
    }
    else
        printf("sn parse failed\r\n");
    sensor__message__free_unpacked(msg, NULL);
    msg = NULL;
    /* send data package then */
    single_phase.currenteffectivevalue = (float)(T1.cur_rms_100x / 100.0);
    single_phase.batteryvoltage = (float)(T1.vol_battery / 100.0);
    single_phase.temperature = (float)(T1.temperature / 100.0);
    single_phase.frequency = (float)50.01;
    single_phase.sn = sn_str;
    single_phase.instantaneouscurrent = wav32_tab;
    init_sensor_msg(&sensor_msg, 2);
    /* send or save to sdcard */
    msg_len = sensor__message__get_packed_size(&sensor_msg);
    sensor__message__pack(&sensor_msg, message_pack_buffer);
    msg2 = sensor__message__unpack(NULL, msg_len, message_pack_buffer);
    printf("\r\nsn: ");
    if(msg2 != NULL)
        printf(((Sensor__Message*)msg2)->singlephase->sn);
    else
        printf("sn parse failed\r\n");
    if(msg2 != NULL)
    {
        sensor__message__free_unpacked(msg2, NULL);
        msg2 = NULL;
        printf("\r\nmsg2 parsed!\r\n");
    }
}

