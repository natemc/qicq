#ifndef QICQ_LAMBDA_H
#define QICQ_LAMBDA_H

////////////////////////////////////////////////////////////////////////////////
// Lambda-generating macros
////////////////////////////////////////////////////////////////////////////////

#ifdef L0
#error "L0 macro conflict"
#elif defined L1
#error "L1 macro conflict"
#elif defined L2
#error "L2 macro conflict"
#elif defined L3
#error "L3 macro conflict"
#elif defined V0
#error "V0 macro conflict"
#elif defined V1
#error "V1 macro conflict"
#elif defined V2
#error "V2 macro conflict"
#elif defined V3
#error "V3 macro conflict"
#else
#define L0(expr) [&](){return expr;}
#define L1(expr) [&](auto&& x){return expr;}
#define L2(expr) [&](auto&& x,auto&& y){return expr;}
#define L3(expr) [&](auto&& x,auto&& y,auto&& z){return expr;}
#define V0(expr) [&](){expr;}
#define V1(expr) [&](auto&& x){expr;}
#define V2(expr) [&](auto&& x,auto&& y){expr;}
#define V3(expr) [&](auto&& x,auto&& y,auto&& z){expr;}
#endif

#endif
