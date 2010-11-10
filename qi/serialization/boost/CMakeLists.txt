set(headers
    boost_serializers.hpp
    boost_binary_serializer.hpp
    boost_binary_serializer.hxx
    boost_text_serializer.hpp
    boost_text_serializer.hxx
    boost_xml_serializer.hpp
    boost_xml_serializer.hxx
)

install_header(LIBIPPC SUBFOLDER qi/serialization/boost ${headers})