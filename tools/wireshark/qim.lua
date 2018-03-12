ip_src_f = Field.new("ip.src")
ip_dst_f = Field.new("ip.dst")
tcp_src_f = Field.new("tcp.srcport")
tcp_dst_f = Field.new("tcp.dstport")
tcp_len_f = Field.new("tcp.len")
tcp_data_f = Field.new("data")

qim_proto = Proto("qim","qi::Messaging")

src_F = ProtoField.string("qim.src","Source")
dst_F = ProtoField.string("qim.dst","Destination")
magic_F = ProtoField.uint32("qim.magic", "Magic", base["HEX"])
id_F = ProtoField.uint32("qim.id","Id")
size_F = ProtoField.uint32("qim.size","Size")
version_F = ProtoField.uint16("qim.version","Version")
type_F = ProtoField.string("qim.type","Type")
flags_F = ProtoField.uint8("qim.flags","Flags")
service_F = ProtoField.uint32("qim.service","Service")
service_desc_F = ProtoField.string("qim.service_description","Service description")
object_F = ProtoField.uint32("qim.object","Object")
action_F = ProtoField.uint32("qim.action","Action")
action_desc_F = ProtoField.string("qim.action_description","Action description")
data_F = ProtoField.bytes("qim.data", "Data")


qim_proto.fields = {src_F, dst_F, magic_F, id_F, size_F, version_F, type_F, flags_F, service_F, service_desc_F, object_F, action_F, action_desc_F, data_F}

function read_buffer(buffer, offset, len)
    return buffer(offset, len), offset + len
end

function qim_proto.dissector(buffer,pinfo,tree)
  local tcp_src = tcp_src_f()
  local tcp_dst = tcp_dst_f()
  local tcp_len = tcp_len_f()
  local tcp_data = tcp_data_f()
  local ip_src = ip_src_f()
  local ip_dst = ip_dst_f()


  if not tcp_src or tonumber(tostring(tcp_len)) < 4 then
    return
  end

  buffer = ByteArray.tvb(tcp_data(0,-1), "TCP data")
  local magic, offset = read_buffer(buffer, 0, 4)

  if tostring(magic) == "42dead42" then
    local subtree = tree:add(qim_proto,"qi::Messaging")
    local src = tostring(ip_src) .. ":" .. tostring(tcp_src)
    local dst = tostring(ip_dst) .. ":" .. tostring(tcp_dst)

    subtree:add(src_F,src)
    subtree:add(dst_F,dst)
    subtree:add(magic_F, magic)

    local id, offset = read_buffer(buffer, offset, 4)
    subtree:add_le(id_F, id)

    local payload_size, offset = read_buffer(buffer, offset, 4)
    subtree:add_le(size_F, payload_size)

    local version, offset = read_buffer(buffer, offset, 2)
    subtree:add_le(version_F, version)

    local typez, offset = read_buffer(buffer, offset, 1)
    typez = typez:le_uint() + 1
    types = { "None", "Call", "Reply", "Error", "Post", "Event", "Capability", "Cancel", "Cancelled" }

    if types[typez] == nil
    then subtree:add(type_F, "Unknown")
    else subtree:add(type_F, types[typez])
    end

    local flags, offset = read_buffer(buffer, offset, 1)
    subtree:add_le(flags_F, flags)

    local service, offset = read_buffer(buffer, offset, 4)
    subtree:add_le(service_F, service)

    local service_uid = service:le_uint() + 1
    services = { "ServiceServer", "ServiceDirectory" }

    if services[service_uid] ~= nil then
        subtree:add(service_desc_F, services[service_uid])
    end

    local object, offset = read_buffer(buffer, offset, 4)
    subtree:add_le(object_F, object)

    local action, offset = read_buffer(buffer, offset, 4)
    subtree:add_le(action_F, action)

    local action_uid = action:le_uint() + 1
    actions = { "registerEvent", "unregisterEvent", "metaObject", "terminate",
        "getProperty", "setProperty", "properties", "registerEventWithSignature" }

    local metaobject_method = 9
    local server_service_uid = 1

    if action_uid == metaobject_method and
        service_uid == server_service_uid then
        subtree:add(action_desc_F, "authenticate")
    elseif actions[action_uid] ~= nil then
        subtree:add(action_desc_F, actions[action_uid])
    end

    local payload, offset = read_buffer(buffer, offset, buffer:len() - offset)
    subtree:add(data_F, payload)
  end
end

register_postdissector(qim_proto)
