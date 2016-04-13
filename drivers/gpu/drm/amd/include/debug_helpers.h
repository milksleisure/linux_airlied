#ifndef DEBUG_HELPERS_H
#define DEBUG_HELPERS_H
/*
 *
 * general debug capabilities
 *
 */
#if defined(CONFIG_DEBUG_KERNEL) || defined(CONFIG_DEBUG_DRIVER)

#if defined(CONFIG_HAVE_KGDB) || defined(CONFIG_KGDB)
#define ASSERT_CRITICAL(expr) do {	\
	if (WARN_ON(!(expr))) { \
		kgdb_breakpoint(); \
	} \
} while (0)
#else
#define ASSERT_CRITICAL(expr) do {	\
	if (WARN_ON(!(expr))) { \
		; \
	} \
} while (0)
#endif

#if defined(CONFIG_DEBUG_KERNEL_DAL)
#define ASSERT(expr) ASSERT_CRITICAL(expr)

#else
#define ASSERT(expr) WARN_ON(!(expr))
#endif

#define BREAK_TO_DEBUGGER() ASSERT(0)

#endif /* CONFIG_DEBUG_KERNEL || CONFIG_DEBUG_DRIVER */


#endif
