#pragma once

#include <iostream>

#include "transport_catalogue.h"
#include "json_reader.h"

namespace protobuf {
	void Serialize(const ::transport::Catalogue& catalog, const ::transport::json::Reader& reader, std::ostream& output);

	void DeSerialize(::transport::Catalogue& new_catalog, ::transport::json::Reader& reader, std::istream& input);
}