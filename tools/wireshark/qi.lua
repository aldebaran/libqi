-- luacheck: globals read Proto ProtoField base dissect_tcp_pdus
-- luacheck: max line length 100

-- Qi Messaging Protocol
-- =====================
local qi = {}

-- Header
-- ------
-- ╔═══════════════════════╤═══════════════════════╤═══════════════════════╤═══════════════════════╗
-- ║           1           │           2           │           3           │           4           ║
-- ╟──┬──┬──┬──┬──┬──┬──┬──┼──┬──┬──┬──┬──┬──┬──┬──┼──┬──┬──┬──┬──┬──┬──┬──┼──┬──┬──┬──┬──┬──┬──┬──╢
-- ║ 0│ 1│ 2│ 3│ 4│ 5│ 6│ 7│ 8│ 9│10│11│12│13│14│15│16│17│18│19│20│21│22│23│24│25│26│27│28│29│30│31║
-- ╠══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╧══╣
-- ║                                                                                               ║
-- ║                                         magic cookie                                          ║
-- ║                                                                                               ║
-- ╟───────────────────────────────────────────────────────────────────────────────────────────────╢
-- ║                                                                                               ║
-- ║                                          identifier                                           ║
-- ║                                                                                               ║
-- ╟───────────────────────────────────────────────────────────────────────────────────────────────╢
-- ║                                                                                               ║
-- ║                                         payload size                                          ║
-- ║                                                                                               ║
-- ╟───────────────────────────────────────────────┬───────────────────────┬─────────────────┬──┬──╢
-- ║                                               │                       │                 │Re│Dy║
-- ║                    version                    │         type          │ flags(reserved) │tT│nP║
-- ║                                               │                       │                 │yp│ay║
-- ╟───────────────────────────────────────────────┴───────────────────────┴─────────────────┴──┴──╢
-- ║                                                                                               ║
-- ║                                            service                                            ║
-- ║                                                                                               ║
-- ╟───────────────────────────────────────────────────────────────────────────────────────────────╢
-- ║                                                                                               ║
-- ║                                            object                                             ║
-- ║                                                                                               ║
-- ╟───────────────────────────────────────────────────────────────────────────────────────────────╢
-- ║                                                                                               ║
-- ║                                            action                                             ║
-- ║                                                                                               ║
-- ╚═══════════════════════════════════════════════════════════════════════════════════════════════╝
--

-- Values
-- ------
qi.values = {
    header = {
        len = 28,
    },
    magiccookie = {
        value = 0x42dead42, -- encoded as big endian in the message.
    },
    type = {
        enum = {
            none       = {id = 0, text = "none"},
            call       = {id = 1, text = "call"},
            reply      = {id = 2, text = "reply"},
            error      = {id = 3, text = "error"},
            post       = {id = 4, text = "post"},
            event      = {id = 5, text = "event"},
            capability = {id = 6, text = "capability"},
            cancel     = {id = 7, text = "cancel"},
            canceled   = {id = 8, text = "canceled"},
        },
    },
    service = {
        enum = {
            server = {id = 0, text = "Server"},
            sd     = {id = 1, text = "ServiceDirectory"},
        },
    },
    object = {
        boundobject_host_ids = {
            min =  0x00000002, -- luacheck: ignore
            max =  0x7fffffff, -- luacheck: ignore
        },
        remoteobject_host_ids = {
            min = 0x80000000, -- luacheck: ignore
            max = 0xffffffff, -- luacheck: ignore
        },
        enum = {
            null                = {id = 0, text = "null"},
            service_main_object = {id = 1, text = "service main object"},
        },
    },
    action = {
        -- The meaning of an action id depends on the service and its object. If the service is
        -- 'Server', then actions must be some special values. The 'ServiceDirectory' service
        -- main object also as some special actions.
        boundobject = {
            enum = {
                register_event = {
                    id = 0,
                    text = "connect to signal (method: registerEvent)"
                },
                unregister_event = {
                    id = 1,
                    text = "disconnect from signal (method: unregisterEvent)"
                },
                metaobject = {
                    id = 2,
                    text = "get the metaobject (method: metaObject)"
                },
                terminate = {
                    id = 3,
                    text = "release object (method: terminate)"
                },
                property = {
                    id = 5, -- no action with id 4 surprisingly
                    text = "get a property (method: property)"
                },
                set_property = {
                    id = 6,
                    text = "set a property (method: setProperty)"
                },
                properties = {
                    id = 7,
                    text = "get property list (method: properties)"
                },
                register_event_with_signature = {
                    id = 8,
                    text = "connect to signal with signature (method: registerEventWithSignature)"
                },
            },
        },
        server = {
            enum = {
                connect      = {id = 4, text = "connect"},
                authenticate = {id = 8, text = "authenticate"},
            }
        },
        sd = {
            enum = {
                service = {
                    id = 100,
                    text = "get a service (method: service)"
                },
                services = {
                    id = 101,
                    text = "get all services (method: services)"
                },
                register_service = {
                    id = 102,
                    text = "register a service (method: registerService)"
                },
                unregister_service = {
                    id = 103,
                    text = "unregister a service (method: unregisterService)"
                },
                service_ready = {
                    id = 104,
                    text = "a service is ready (method: serviceReady)"
                },
                update_service_info = {
                    id = 105,
                    text = "update information of a service (method: updateServiceInfo)"},
                service_added = {
                    id = 106,
                    text = "a service has been added (signal: serviceAdded)"
                },
                service_removed = {
                    id = 107,
                    text = "a service has been removed (signal: serviceRemoved)"
                },
                machine_id = {
                    id = 108,
                    text = "get the machine id (method: machineId)"
                },
            },
        },
    },
}

-- ServiceDirectory specific actions include the BoundObject actions.
for k,v in pairs(qi.values.action.boundobject.enum) do
    qi.values.action.sd.enum[k] = v
end

-- Transforms enumerated values into association tables (id_to_text and text_to_id).
-- There might be a better way to do this, but this works good enough for now.
for _,t in ipairs({qi.values.type,
                   qi.values.service,
                   qi.values.object,
                   qi.values.action.boundobject,
                   qi.values.action.server,
                   qi.values.action.sd}) do
    t.id_to_text = {}
    t.text_to_id = {}
    if t.enum then
        for _, value in pairs(t.enum) do
            assert(value.id and value.text)
            t.id_to_text[value.id] = value.text
            t.text_to_id[value.text] = value.id
        end
    end
end

-- Returns a ProtoField for the action header field with custom values.
function qi.make_field_action(valuestrings)
    return ProtoField.uint32("qi.action", "Action", base.DEC, valuestrings)
end


-- Fields
-- ------
qi.fields = {
    src = {
        proto = ProtoField.string("qi.src", "Source"),
    },
    dst = {
        proto = ProtoField.string("qi.dst", "Destination"),
    },
    magiccookie = {
        proto = ProtoField.uint32("qi.magiccookie", "Magic cookie", base.HEX),
        len = 4, -- bytes for an uint32
    },
    id = {
        proto = ProtoField.uint32("qi.id", "Id"),
        len = 4, -- bytes for an uint32
    },
    payload_len = {
        proto = ProtoField.uint32("qi.len", "Len"),
        len = 4, -- bytes for an uint32
    },
    version = {
        proto = ProtoField.uint16("qi.version", "Version"),
        len = 2, -- bytes for an uint16
    },
    type = {
        proto = ProtoField.uint8("qi.type", "Type", base.DEC, qi.values.type.id_to_text),
        len = 1, -- byte for an uint8,
    },
    flags = {
        proto = ProtoField.uint8("qi.flags", "Flags", base.HEX),
        len = 1, -- 1 byte for an uint8
        dynamicpayload = {
            proto = ProtoField.bool("qi.flags.dynamicpayload", "DynamicPayload", 8, nil, 0x01),
        },
        returntype = {
            proto = ProtoField.bool("qi.flags.returntype", "ReturnType", 8, nil, 0x02),
        },
    },
    service = {
        proto = ProtoField.uint32("qi.service", "Service", base.DEC, qi.values.service.id_to_text),
        len = 4, -- bytes for an uint32
    },
    object = {
        proto = ProtoField.uint32("qi.object", "Object", base.DEC_HEX,
                                    qi.values.object.id_to_text),
        len = 4, -- bytes for an uint32
    },
    action = {
        -- Same field, different values that depend on the service and object.
        boundobject = { proto = qi.make_field_action(qi.values.action.boundobject.id_to_text), },
        server      = { proto = qi.make_field_action(qi.values.action.server.id_to_text), },
        sd          = { proto = qi.make_field_action(qi.values.action.sd.id_to_text), },
        len = 4, -- bytes for an uint32
    },
    payload = {
        proto = ProtoField.bytes("qi.payload", "Payload", base.SPACE)
    },
}

assert(qi.values.header.len ==
    qi.fields.magiccookie.len
        + qi.fields.id.len
        + qi.fields.payload_len.len
        + qi.fields.version.len
        + qi.fields.type.len
        + qi.fields.flags.len
        + qi.fields.service.len
        + qi.fields.object.len
        + qi.fields.action.len)


-- Proto
-- -----
qi.proto = Proto("QI", "Qi Messaging Protocol")

qi.proto.fields = {
    qi.fields.src.proto,
    qi.fields.dst.proto,
    qi.fields.magiccookie.proto,
    qi.fields.id.proto,
    qi.fields.payload_len.proto,
    qi.fields.version.proto,
    qi.fields.type.proto,
    qi.fields.flags.proto,
    qi.fields.flags.dynamicpayload.proto,
    qi.fields.flags.returntype.proto,
    qi.fields.service.proto,
    qi.fields.object.proto,
    qi.fields.action.boundobject.proto,
    qi.fields.action.sd.proto,
    qi.fields.action.server.proto,
    qi.fields.payload.proto,
}

-- Splits a buffer in two at the given position and returns the two smaller parts.
-- If the position equals the length of the buffer, returns the buffer and an empty buffer.
-- If the position is negative or greater than the length of the buffer, returns nothing (`nil`).
function qi.split(buffer, pos)
    assert(buffer)
    assert(pos)
    local len = buffer:len()
    if pos == len then
        return buffer, buffer(0, 0)
    elseif pos >= 0 and pos < len then
        return buffer(0, pos), buffer(pos)
    end
end

-- Returns true if the buffer starts with a Qi magic cookie, otherwise returns false.
function qi.msg_starts_with_magiccookie(buffer)
    assert(buffer)
    if buffer:len() < qi.fields.magiccookie.len then
        return false
    end
    return buffer(0, qi.fields.magiccookie.len):uint() == qi.values.magiccookie.value
end

-- Dissects the header information from a buffer.
-- Returns a table containing the buffer and all the header fields information.
-- Each field information consists of a buffer, a value and a text representing
-- the value when available.
function qi.dissect_msg_header(buffer)
    assert(buffer)
    assert(buffer:len() >= qi.values.header.len)

    local header_buffer = buffer

    -- magiccookie
    local magiccookie_buffer
    magiccookie_buffer, buffer = qi.split(buffer, qi.fields.magiccookie.len)
    local magiccookie = {
        buffer = magiccookie_buffer,
        value = magiccookie_buffer:uint()
    }

    -- id, uint32, little endian
    local id_buffer
    id_buffer, buffer = qi.split(buffer, qi.fields.id.len)
    local id = {
        buffer = id_buffer,
        value = id_buffer:le_uint()
    }

    -- size/len, uint32, little endian
    local payload_len_buffer
    payload_len_buffer, buffer = qi.split(buffer, qi.fields.payload_len.len)
    local payload_len = {
        buffer = payload_len_buffer,
        value = payload_len_buffer:le_uint()
    }

    -- version, uint16, little endian
    local version_buffer
    version_buffer, buffer = qi.split(buffer, qi.fields.version.len)
    local version = {
        buffer = version_buffer,
        value = version_buffer:le_uint()
    }

    -- type, uint8, little endian
    local type_buffer
    type_buffer, buffer = qi.split(buffer, qi.fields.type.len)
    local type_id = type_buffer:le_uint()
    local msg_type = {  -- 'type' is a reserved keyword
        buffer = type_buffer,
        value = type_id,
        text = qi.values.type.id_to_text[type_id] or error("Unknown message type.")
    }

    -- flags, uint8, little endian
    local flags_buffer
    flags_buffer, buffer = qi.split(buffer, qi.fields.flags.len)
    local flags = {
        buffer = flags_buffer,
        value = flags_buffer:bitfield()
    }

    -- service, uint32, little endian
    local service_buffer
    service_buffer, buffer = qi.split(buffer, qi.fields.service.len)
    local service_id = service_buffer:le_uint()
    local service = {
        buffer = service_buffer,
        value = service_id,
        text = qi.values.service.id_to_text[service_id]
    }

    -- object, uint32, little endian
    local object_buffer
    object_buffer, buffer = qi.split(buffer, qi.fields.object.len)
    local object_id = object_buffer:le_uint()
    local object = {
        buffer = object_buffer,
        value = object_id,
        text = qi.values.object.id_to_text[object_id]
    }

    -- action, uint32, little endian
    local action_buffer
    action_buffer = qi.split(buffer, qi.fields.action.len)
    local action_id = action_buffer:le_uint()
    local action_text
    -- Actions concerning the 'Server' service must target the 'null' object.
    if service_id == qi.values.service.enum.server.id
        and object_id == qi.values.object.enum.null.id then
        action_text = qi.values.action.server.id_to_text[action_id]
    -- Actions concerning the 'ServiceDirectory' only have special values if
    -- they target the service main object.
    elseif service_id == qi.values.service.enum.sd.id
        and object_id == qi.values.object.enum.service_main_object.id then
        action_text = qi.values.action.sd.id_to_text[action_id]
    else
        action_text = qi.values.action.boundobject.id_to_text[action_id]
    end
    local action = {
        buffer = action_buffer,
        value = action_id,
        text = action_text
    }

    return {
        buffer = header_buffer,
        magiccookie = magiccookie,
        id = id,
        payload_len = payload_len,
        version = version,
        type = msg_type,
        flags = flags,
        service = service,
        object = object,
        action = action
    }
end

-- Dissects one message information from a buffer.
-- Returns a table containing the message buffer, the header information and the payload
-- information which consists of a buffer.
function qi.dissect_msg(buffer)
    assert(buffer)
    assert(buffer:len() >= qi.values.header.len)

    local msg_buffer = buffer

    local header_buffer
    header_buffer, buffer = qi.split(buffer, qi.values.header.len)
    local header = qi.dissect_msg_header(header_buffer)

    -- Some messages don't have a payload.
    local payload_buffer
    if header.payload_len.value > 0 then
        assert(buffer:len() >= header.payload_len.value)
        payload_buffer = buffer(0, header.payload_len.value)
    end
    local payload = {
        buffer = payload_buffer
    }

    return {
        buffer = msg_buffer,
        header = header,
        payload = payload
    }
end

-- Returns a string summarizing the message header information.
function qi.msg_summary(msg)
    local header = msg.header
    return "[" .. (header.type.text or header.type.value) .. "] "
        .. header.id.value .. "."
        .. header.service.value .. "."
        .. header.object.value .. "."
        .. header.action.value
end

-- Creates a new item in the dissection tree representing the message.
function qi.add_msg_pinfo_treeitem(pinfo, tree, msg)
    assert(pinfo)
    assert(tree)
    assert(msg)

    local src_str = tostring(pinfo.src) .. ":" .. pinfo.src_port
    local dst_str = tostring(pinfo.dst) .. ":" .. pinfo.dst_port
    local summary = qi.msg_summary(msg)

    -- Set the packet information and protocol name.
    local prefix
    if tostring(pinfo.cols.protocol) == qi.proto.name then
        prefix = tostring(pinfo.cols.info)
    else
        prefix = src_str .. " → " .. dst_str
    end
    pinfo.cols.info:set(prefix ..  ", " .. summary)
    pinfo.cols.protocol:set(qi.proto.name)

    local msg_item = tree:add(qi.proto, msg.buffer):append_text(", " .. summary)

    msg_item:add(qi.fields.src.proto, src_str):set_generated(true)
    msg_item:add(qi.fields.dst.proto, dst_str):set_generated(true)

    local header = msg.header

    msg_item:add(qi.fields.magiccookie.proto, header.magiccookie.buffer):set_hidden(true)
    msg_item:add_le(qi.fields.id.proto, header.id.buffer)
    msg_item:add_le(qi.fields.payload_len.proto, header.payload_len.buffer)
    msg_item:add_le(qi.fields.version.proto, header.version.buffer)
    msg_item:add_le(qi.fields.type.proto, header.type.buffer)

    -- The flag item has some children items to have a nice tree view of all the bitfield values.
    local flags_item = msg_item:add(qi.fields.flags.proto, header.flags.buffer)
    flags_item:add_le(qi.fields.flags.dynamicpayload.proto, header.flags.buffer)
    flags_item:add_le(qi.fields.flags.returntype.proto, header.flags.buffer)

    msg_item:add_le(qi.fields.service.proto, header.service.buffer)
    msg_item:add_le(qi.fields.object.proto, header.object.buffer)

    -- If the object is hosted by a RemoteObject, add a translation of the id as an offset from
    -- the minimum RemoteObject hosted object id (it is easier to read and follow around).
    if header.object.value >= qi.values.object.remoteobject_host_ids.min
        and header.object.value <= qi.values.object.remoteobject_host_ids.max then
        msg_item:add(header.object.buffer,
            "Hosted by a RemoteObject, translated object id: "
                .. (header.object.value - qi.values.object.remoteobject_host_ids.min))
            :set_generated(true)
    end

    -- Interpret the action depending on the target service and object.
    if header.service.value == qi.values.service.enum.server.id then
        msg_item:add_le(qi.fields.action.server.proto, header.action.buffer)
    elseif header.service.value == qi.values.service.enum.sd.id then
        msg_item:add_le(qi.fields.action.sd.proto, header.action.buffer)
    else
        msg_item:add_le(qi.fields.action.boundobject.proto, header.action.buffer)
    end

    if msg.payload.buffer then
        msg_item:add(qi.fields.payload.proto, msg.payload.buffer)
    end
end

-- The main dissector function that takes the packet Tvb buffer, its information and the
-- dissection tree.
-- Returns true if the packet starts as a Qi message packet (with a magic cookie).
function qi.proto.dissector(tvb, _, tree)
    if not qi.msg_starts_with_magiccookie(tvb) then
        return false
    end

    -- Returns the total length of the message at the offset position in the Tvb buffer.
    -- luacheck: ignore 432
    local function get_msg_len(tvb, _, offset)
        local buffer = tvb(offset)
        local msg_len = qi.values.header.len + qi.dissect_msg_header(buffer).payload_len.value
        return msg_len
    end

    -- Dissects a message in the Tvb buffer, update the packet information and adds it to the
    -- dissection tree.
    -- Returns the total number of bytes processed.
    local function process_msg(tvb, pinfo, tree)
        local buffer = tvb()
        local msg = qi.dissect_msg(buffer)
        qi.add_msg_pinfo_treeitem(pinfo, tree, msg)
        return qi.values.header.len + msg.header.payload_len.value
    end

    -- This function provided by the Wireshark TCP dissector enables us to automatically reconstruct
    -- the Qi protocol data units (PDU), aka messages. It takes care of splitting or reassembling
    -- TCP packets into a single data unit that we can then process.
    -- It requires that each PDU of the protocol starts with a fixed size header that contains a
    -- field allowing the computation of the full length of the PDU, which is the case for the Qi
    -- protocol.
    dissect_tcp_pdus(
        tvb, tree,
        qi.values.header.len, -- The header fixed size
        get_msg_len, -- The function that is called to compute the full length of the PDU.
        process_msg -- The function that is called to dissect one PDU.
    )
    return true
end

-- Registers the Qi dissector as an heuristic for TCP packets. It requires that the dissector
-- function return true if it is a packet for the Qi protocol and false otherwise.
-- This is perfect for the Qi protocol because our heuristic is simple: each message must start
-- with a magic cookie.
qi.proto:register_heuristic("tcp", qi.proto.dissector)
