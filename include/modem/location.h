/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef LOCATION_H_
#define LOCATION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#if defined(CONFIG_LOCATION_METHOD_GNSS_AGPS_EXTERNAL) || defined(CONFIG_LOCATION_DATA_DETAILS)
#include <nrf_modem_gnss.h>
#endif
#if defined(CONFIG_LOCATION_METHOD_GNSS_PGPS_EXTERNAL)
#include <net/nrf_cloud_pgps.h>
#endif
#if defined(CONFIG_LOCATION_METHOD_CELLULAR_EXTERNAL)
#include <modem/lte_lc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file location.h
 * @brief Public APIs for the Location library.
 * @defgroup location Location library
 * @{
 */

/** Location method. */
enum location_method {
	/** LTE cellular positioning. */
	LOCATION_METHOD_CELLULAR = 1,
	/** Global Navigation Satellite System (GNSS). */
	LOCATION_METHOD_GNSS,
	/** Wi-Fi positioning. */
	LOCATION_METHOD_WIFI,
};

/** Location acquisition mode. */
enum location_req_mode {
	/** Fallback to next preferred method (default). */
	LOCATION_REQ_MODE_FALLBACK = 0,
	/** All requested methods are used sequentially. */
	LOCATION_REQ_MODE_ALL,
};

/** Event IDs. */
enum location_event_id {
	/** Location update. */
	LOCATION_EVT_LOCATION = 1,
	/** Getting location timed out. */
	LOCATION_EVT_TIMEOUT,
	/** An error occurred when trying to get the location. */
	LOCATION_EVT_ERROR,
	/**
	 * Application has indicated that getting location has been completed,
	 * the result is not known, and the Location library does not need to care about it.
	 * This event can occur only if CONFIG_LOCATION_METHOD_CELLULAR_EXTERNAL is set.
	 */
	LOCATION_EVT_RESULT_UNKNOWN,
	/**
	 * GNSS is requesting A-GPS data. Application should obtain the data and send it to
	 * location_agps_data_process().
	 */
	LOCATION_EVT_GNSS_ASSISTANCE_REQUEST,
	/**
	 * GNSS is requesting P-GPS data. Application should obtain the data and send it to
	 * location_pgps_data_process().
	 */
	LOCATION_EVT_GNSS_PREDICTION_REQUEST,
	/**
	 * Cellular location request with neighbor cell information is available.
	 * Application should send the cell information to cloud services and
	 * then call location_cellular_ext_result_set().
	 */
	LOCATION_EVT_CELLULAR_EXT_REQUEST
};

/** Result of the external cellular request. */
enum location_cellular_ext_result {
	/* Cellular location result is successful. */
	LOCATION_CELLULAR_EXT_RESULT_SUCCESS,
	/* Cellular location result is unknown. */
	LOCATION_CELLULAR_EXT_RESULT_UNKNOWN,
	/* Cellular location result is error. */
	LOCATION_CELLULAR_EXT_RESULT_ERROR
};

/** Location accuracy. */
enum location_accuracy {
	/** Allow lower accuracy to conserve power. */
	LOCATION_ACCURACY_LOW,
	/** Normal accuracy. */
	LOCATION_ACCURACY_NORMAL,
	/** Try to improve accuracy. Increases power consumption. */
	LOCATION_ACCURACY_HIGH,
};

/** Location service. */
enum location_service {
	/**
	 * Use any location service that has been configured to be available.
	 * This is useful when only one service is configured but can also be used
	 * even if several services are available.
	 */
	LOCATION_SERVICE_ANY,
	/** nRF Cloud location service. */
	LOCATION_SERVICE_NRF_CLOUD,
	/** HERE location service. */
	LOCATION_SERVICE_HERE
};

/** Date and time (UTC). */
struct location_datetime {
	/** True if date and time are valid, false if not. */
	bool valid;
	/** 4-digit representation (Gregorian calendar). */
	uint16_t year;
	/** 1...12 */
	uint8_t month;
	/** 1...31 */
	uint8_t day;
	/** 0...23 */
	uint8_t hour;
	/** 0...59 */
	uint8_t minute;
	/** 0...59 */
	uint8_t second;
	/** 0...999 */
	uint16_t ms;
};

#if defined(CONFIG_LOCATION_DATA_DETAILS)
/** Location details for GNSS. */
struct location_data_details_gnss {
	/** Number of satellites tracked at the time of event. */
	uint8_t satellites_tracked;
	/** PVT data. */
	struct nrf_modem_gnss_pvt_data_frame pvt_data;
};

/** Location details. */
struct location_data_details {
	/** Location details for GNSS. */
	struct location_data_details_gnss gnss;
};
#endif

/** Location data. */
struct location_data {
	/** Geodetic latitude (deg) in WGS-84. */
	double latitude;
	/** Geodetic longitude (deg) in WGS-84. */
	double longitude;
	/** Location accuracy in meters (1-sigma). */
	float accuracy;
	/** Date and time (UTC). */
	struct location_datetime datetime;

#if defined(CONFIG_LOCATION_DATA_DETAILS)
	/** Location details. */
	struct location_data_details details;
#endif
};

#if defined(CONFIG_LOCATION_DATA_DETAILS)
/** Location error information. */
struct location_data_error {
	/** Data details at the time of error. */
	struct location_data_details details;
};
#endif

/** Location event data. */
struct location_event_data {
	/** Event ID. */
	enum location_event_id id;
	/** Used location method. */
	enum location_method method;

	union {
		/** Current location, used with event LOCATION_EVT_LOCATION. */
		struct location_data location;

#if defined(CONFIG_LOCATION_DATA_DETAILS)
		/**
		 * Relevant location data when a timeout or an error occurs.
		 * Used with event LOCATION_EVT_TIMEOUT and LOCATION_EVT_ERROR.
		 */
		struct location_data_error error;
#endif

#if defined(CONFIG_LOCATION_METHOD_GNSS_AGPS_EXTERNAL)
		/**
		 * A-GPS notification data frame used by GNSS to let the application know it
		 * needs new assistance data, used with event LOCATION_EVT_GNSS_ASSISTANCE_REQUEST.
		 */
		struct nrf_modem_gnss_agps_data_frame agps_request;
#endif
#if  defined(CONFIG_LOCATION_METHOD_GNSS_PGPS_EXTERNAL)
		/**
		 * P-GPS notification data frame used by GNSS to let the application know it
		 * needs new assistance data, used with event LOCATION_EVT_GNSS_PREDICTION_REQUEST.
		 */
		struct gps_pgps_request pgps_request;
#endif
#if  defined(CONFIG_LOCATION_METHOD_CELLULAR_EXTERNAL)
		/**
		 * Cellular cell information to let the application know it should send these
		 * to a cloud service to resolve the location.
		 * Used with event LOCATION_EVT_CELLULAR_EXT_REQUEST.
		 */
		struct lte_lc_cells_info cellular_request;
#endif
	};
};

/** GNSS configuration. */
struct location_gnss_config {
	/**
	 * @brief Timeout (in milliseconds), meaning how long GNSS is allowed to run when trying to
	 * acquire a fix. SYS_FOREVER_MS means that the timer is disabled, meaning that GNSS will
	 * continue to search until it gets a fix or the application calls cancel.
	 *
	 * @details Note that this is not real time as experienced by the user.
	 * Since GNSS cannot run while LTE is operating, the running time is counted from
	 * the instant when GNSS is allowed to start. Library waits until RRC connection
	 * is inactive before starting GNSS. If LTE power saving mode (PSM) is enabled
	 * and A-GPS is disabled, library waits until modem enters PSM before starting GNSS,
	 * thus maximizing uninterrupted operating window and minimizing power consumption.
	 */
	int32_t timeout;

	/** @brief Desired accuracy level.
	 *
	 * @details If accuracy is set to LOCATION_ACCURACY_LOW, GNSS relaxes the
	 * criteria for a qualifying fix and may produce fixes using only three satellites.
	 *
	 * If accuracy is set to LOCATION_ACCURACY_HIGH, instead of using the first fix, GNSS is
	 * allowed to run for a longer time. This typically improves the location accuracy.
	 */
	enum location_accuracy accuracy;

	/**
	 * @brief The number of fixes GNSS is allowed to produce before the library outputs the
	 * current location when accuracy is set to LOCATION_ACCURACY_HIGH.
	 *
	 * @details If accuracy is set to LOCATION_ACCURACY_NORMAL or LOCATION_ACCURACY_LOW this
	 * parameter has no effect.
	 */
	uint8_t num_consecutive_fixes;

	/**
	 * @brief Obstructed visibility detection. If set to true, the library tries to detect
	 * situations where getting a GNSS fix is unlikely or would consume a lot of energy.
	 * When such a situation is detected, GNSS is stopped without waiting for a fix or a
	 * timeout.
	 *
	 * @details See Kconfig for related configuration options.
	 *
	 * @note Only supported with modem firmware v1.3.2 or later.
	 */
	bool visibility_detection;

	/**
	 * @brief Enable GNSS priority mode if GNSS does not get enough runtime due to LTE idle mode
	 * operations.
	 *
	 * @details If set to true, the library triggers GNSS priority mode if five consecutive PVT
	 * messages indicate that GNSS is blocked by LTE idle mode operations. This is especially
	 * helpful if A-GPS or P-GPS is not enabled or downloading assistance data fails and GNSS
	 * module has to decode navigation data from the satellite broadcast. Priority mode is
	 * disabled automatically after the first fix or after 40 seconds.
	 *
	 * If the device attempts to send data during the priority mode, it will be buffered and
	 * sent after the priority time window ends. In case of mobile terminated data reception
	 * during the priority mode the network will typically buffer the data and sent them to the
	 * device once the priority time window ends. However, it is possible that the network drops
	 * the data, or some protocol timer expires causing data transfer to fail.
	 */
	bool priority_mode;
};

/** LTE cellular positioning configuration. */
struct location_cellular_config {
	/**
	 * @brief Timeout (in milliseconds) of how long the cellular positioning procedure can take.
	 * SYS_FOREVER_MS means that the timer is disabled.
	 */
	int32_t timeout;

	/** Used cellular positioning service. */
	enum location_service service;
};

/** Wi-Fi positioning configuration. */
struct location_wifi_config {
	/**
	 * @brief Timeout (in milliseconds) of how long the Wi-Fi positioning procedure can take.
	 * SYS_FOREVER_MS means that the timer is disabled.
	 */
	int32_t timeout;

	/** Used Wi-Fi positioning service. */
	enum location_service service;
};

/** Location method configuration. */
struct location_method_config {
	/** Location method. */
	enum location_method method;
	union {
		/** Configuration for LOCATION_METHOD_CELLULAR. */
		struct location_cellular_config cellular;
		/** Configuration for LOCATION_METHOD_GNSS. */
		struct location_gnss_config gnss;
		/** Configuration for LOCATION_METHOD_WIFI. */
		struct location_wifi_config wifi;
	};
};

/** Location request configuration. */
struct location_config {
	/** Number of location methods in 'methods'. */
	uint8_t methods_count;
	/**
	 * @brief Selected location methods and associated configurations in priority order.
	 *
	 * @details Index 0 has the highest priority. Number of used methods is indicated in
	 * 'methods_count' and it can be smaller than the size of this table.
	 */
	struct location_method_config methods[CONFIG_LOCATION_METHODS_LIST_SIZE];
	/**
	 * @brief Position update interval in seconds.
	 *
	 * @details Set to 0 for a single position update. For periodic position updates
	 * the valid range is 1...65535 seconds.
	 */
	uint16_t interval;
	/**
	 * @brief Timeout (in milliseconds) for the entire location request.
	 * SYS_FOREVER_MS means that the timer is disabled.
	 */
	int32_t timeout;

	/**
	 * @brief Location acquisition mode.
	 */
	enum location_req_mode mode;
};

/**
 * @brief Event handler prototype.
 *
 * @param[in] event_data Event data.
 */
typedef void (*location_event_handler_t)(const struct location_event_data *event_data);

/**
 * @brief Initializes the library.
 *
 * @details Initializes the library and sets the event handler function.
 * The first call to this function must be called before going to LTE normal mode.
 * This can be called multiple times, which sets the event handler again.
 *
 * @param[in] event_handler Event handler function.
 *
 * @return 0 on success, or negative error code on failure.
 * @retval -EINVAL Given event_handler is NULL.
 * @retval -EFAULT Failed to obtain Wi-Fi interface.
 */
int location_init(location_event_handler_t event_handler);

/**
 * @brief Requests the current position or starts periodic position updates.
 *
 * @details Requests the current position using the given configuration. Depending on the
 * configuration, a single position or periodic position updates are given. If the position
 * request is successful, the results are delivered to the event handler function in
 * LOCATION_EVT_LOCATION event. Otherwise LOCATION_EVT_TIMEOUT or LOCATION_EVT_ERROR event is
 * triggered.
 *
 * In periodic mode, position update is always attempted after the configured interval
 * regardless of the outcome of any previous attempt. Periodic position updates can be
 * stopped by calling location_cancel().
 *
 * @param[in] config Used configuration or NULL to get a single position update with
 *                   the default configuration. Default configuration has the following
 *                   location methods in priority order (if they are enabled in library
 *                   configuration): GNSS, Wi-Fi and Cellular.
 *
 * @return 0 on success, or negative error code on failure.
 * @retval -EPERM  Library not initialized.
 * @retval -EBUSY  Previous location request still ongoing.
 * @retval -EINVAL Invalid configuration given.
 */
int location_request(const struct location_config *config);

/**
 * @brief Cancels periodic position updates.
 *
 * @return 0 on success, or negative error code on failure.
 * @retval -EPERM  Library not initialized.
 */
int location_request_cancel(void);

/**
 * @brief Sets the default values to a given configuration.
 *
 * @param[in,out] config Configuration to be supplied with default values.
 * @param[in] methods_count Number of location methods. This must not exceed
 *                          CONFIG_LOCATION_METHODS_LIST_SIZE.
 * @param[in] method_types List of location method types in priority order.
 *                         Location methods with these types are initialized with default values
 *                         into given configuration. List size must be as given in 'methods_count'.
 */
void location_config_defaults_set(
	struct location_config *config,
	uint8_t methods_count,
	enum location_method *method_types);

/**
 * @brief Return location method as a string.
 *
 * @param[in] method Location method.
 *
 * @return Location method in string format. Returns "Unknown" if given method is not known.
 */
const char *location_method_str(enum location_method method);

/**
 * @brief Feed in A-GPS data to be processed by library.
 *
 * @details If Location library is not receiving A-GPS assistance data directly from nRF Cloud,
 * it triggers the LOCATION_EVT_GNSS_ASSISTANCE_REQUEST event when assistance is needed. Once
 * application has obtained the assistance data it can be handed over to Location library using this
 * function.
 *
 * Note that the data must be formatted similarly to the nRF Cloud A-GPS data, see for example
 * nrf_cloud_agps_schema_v1.h.
 *
 * @param[in] buf Data received.
 * @param[in] buf_len Buffer size of data to be processed.
 *
 * @return 0 on success, or negative error code on failure.
 * @retval -EINVAL Given buffer is NULL or buffer length zero.
 * @retval -ENOTSUP CONFIG_LOCATION_GNSS_AGPS_EXTERNAL is not set.
 */
int location_agps_data_process(const char *buf, size_t buf_len);

/**
 * @brief Feed in P-GPS data to be processed by library.
 *
 * @details If Location library is not receiving P-GPS assistance data directly from nRF Cloud,
 * it triggers the LOCATION_EVT_GNSS_PREDICTION_REQUEST event when assistance is needed. Once
 * application has obtained the assistance data it can be handed over to Location library using this
 * function.
 *
 * Note that the data must be formatted similarly to the nRF Cloud P-GPS data, see for example
 * nrf_cloud_pgps_schema_v1.h.
 *
 * @param[in] buf Data received.
 * @param[in] buf_len Buffer size of data to be processed.
 *
 * @return 0 on success, or negative error code on failure.
 * @retval -EINVAL Given buffer is NULL or buffer length zero.
 * @retval -ENOTSUP CONFIG_LOCATION_GNSS_PGPS_EXTERNAL is not set.
 */
int location_pgps_data_process(const char *buf, size_t buf_len);

/**
 * @brief Pass cellular location result to the library.
 *
 * @details If the Location library is not receiving cellular position directly from services,
 * it triggers the @ref LOCATION_EVT_CELLULAR_EXT_REQUEST event when neighbor cell information
 * should be sent to cloud services for location resolution. Then, the application responds
 * with the result of the location result.
 *
 * In addition to successful and error results, the application can indicate an unknown result with
 * @ref LOCATION_CELLULAR_EXT_RESULT_UNKNOWN. This is useful when the application wants
 * the Location library to proceed irrespective of the outcome. The Location library will try to
 * perform a fallback to the next method, if available, just like in a failure case.
 * If there are no more methods, LOCATION_EVT_RESULT_UNKNOWN event will be sent to the application.
 *
 * @param[in] result Result of the external cellular request.
 * @param[in] location Cellular location data. Will be used only if @p result is
 *                     LOCATION_CELLULAR_EXT_RESULT_SUCCESS.
 */
void location_cellular_ext_result_set(
	enum location_cellular_ext_result result,
	struct location_data *location);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LOCATION_H_ */
