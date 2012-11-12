ip_src_f = Field.new("ip.src")
ip_dst_f = Field.new("ip.dst")
tcp_src_f = Field.new("tcp.srcport")
tcp_dst_f = Field.new("tcp.dstport")
tcp_len_f = Field.new("tcp.len")
tcp_data_f = Field.new("data")

qim_proto = Proto("qim","qi::Messaging")

src_F = ProtoField.string("qim.src","Source")
dst_F = ProtoField.string("qim.dst","Destination")
magic_F = ProtoField.uint32("qim.magic", "Magic", base.HEX)
id_F = ProtoField.uint32("qim.id","Id")
size_F = ProtoField.uint32("qim.size","Size")
version_F = ProtoField.uint16("qim.version","Version")
type_F = ProtoField.uint16("qim.type","Type")
service_F = ProtoField.uint32("qim.service","Service")
object_F = ProtoField.uint32("qim.object","Object")
action_F = ProtoField.uint32("qim.action","Action")
data_F = ProtoField.string("qim.data", "Data")


qim_proto.fields = {src_F, dst_F, conv_F, magic_F, id_F, size_F, type_F, service_F, object_F, action_F, data_F}

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

  offset = 68
  local magic = buffer(offset, 4)
  offset = offset + 4

  if tostring(magic) == "42dead42" then
    local subtree = tree:add(qim_proto,"qi::Messaging")
    local src = tostring(ip_src) .. ":" .. tostring(tcp_src)
    local dst = tostring(ip_dst) .. ":" .. tostring(tcp_dst)

    subtree:add(src_F,src)
    subtree:add(dst_F,dst)
    subtree:add(magic_F, magic)

    local id = buffer(offset, 4)
    offset = offset + 4
    subtree:add_le(id_F, id)

    local size = buffer(offset, 4)
    offset = offset + 4
    subtree:add_le(size_F, size)

    local version = buffer(offset, 2)
    offset = offset + 2
    subtree:add_le(version_F, version)

    local typez = buffer(offset, 2)
    offset = offset + 2
    subtree:add_le(type_F, typez)

    local service = buffer(offset, 4)
    offset = offset + 4
    subtree:add_le(service_F, service)

    local object = buffer(offset, 4)
    offset = offset + 4
    subtree:add_le(object_F, object)

    local action = buffer(offset, 4)
    offset = offset + 4
    subtree:add_le(action_F, action)

    local data = buffer(offset, buffer:len() - offset)
    subtree:add(data_F, tostring(data))
  end
end

register_postdissector(qim_proto)
