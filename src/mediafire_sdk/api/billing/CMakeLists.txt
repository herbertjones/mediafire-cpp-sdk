
set(API_SOURCES
    ${API_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/cancel_plan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_invoice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_plans.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_products.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/purchase_plan.cpp
    PARENT_SCOPE
)

set(API_HEADERS
    ${API_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/cancel_plan.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_invoice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_plans.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/get_products.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/purchase_plan.hpp
    PARENT_SCOPE
)

