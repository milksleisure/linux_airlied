#ifndef __DISPLAY_TYPES_H__
#define __DISPLAY_TYPES_H__

enum dce_version {
	DCE_VERSION_UNKNOWN = (-1),
#if defined(CONFIG_DRM_AMD_DAL_DCE8_0)
	DCE_VERSION_8_0,
#endif
#if defined(CONFIG_DRM_AMD_DAL_DCE10_0)
	DCE_VERSION_10_0,
#endif
#if defined(CONFIG_DRM_AMD_DAL_DCE11_0)
	DCE_VERSION_11_0,
#endif
#if defined(CONFIG_DRM_AMD_DAL_DCE11_2)
	DCE_VERSION_11_2,
#endif
	DCE_VERSION_MAX,
};
#endif
