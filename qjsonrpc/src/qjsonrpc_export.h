#ifndef QJSONRPC_EXPORT_H
#define QJSONRPC_EXPORT_H

#ifdef QJSONRPC_SHARED
#   ifdef QJSONRPC_BUILD
#       define QJSONRPC_EXPORT Q_DECL_EXPORT
#   else
#       define QJSONRPC_EXPORT Q_DECL_IMPORT
#   endif
#else
#   define QJSONRPC_EXPORT
#endif

#endif
