#include <netinet/in.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <proto.pb.h>
#include <riemanntcpconnection.h>

namespace {
  char ok_response[1024];
  uint32_t ok_response_size = 0;
}

static void generate_msg_ok()
{
  Msg msg_ok;
  msg_ok.set_ok(true);

  uint32_t nsize = htonl(msg_ok.ByteSize());
  memcpy(ok_response, (void *)&nsize, sizeof(nsize));
  if (!msg_ok.SerializeToArray(ok_response + sizeof(nsize),
                               msg_ok.ByteSize()))
  {
    VLOG(1) << "Error serializing response\n";
    exit(1);
  }

  ok_response_size = sizeof(nsize) + msg_ok.ByteSize();
}

riemann_tcp_connection::riemann_tcp_connection(
  int sfd,
  incoming_events& income_events
) :
  tcp_connection(sfd),
  reading_header(true),
  protobuf_size(0),
  income_events(income_events)
{
  io.set(ev::READ);

  // FIXME
  if (ok_response_size == 0) {
    generate_msg_ok();
  }
  memcpy((void*)&w_buffer, ok_response, ok_response_size);
}

void riemann_tcp_connection::callback(int revents) {
  VLOG(3) << "callback() ";

  if (revents & EV_READ) {
    VLOG(3) << "entering read_cb()";
    read_cb();
  }

  if (close_connection) {
    VLOG(3) << "close_connection";
    return;
  }

  if (revents & EV_WRITE) {
    VLOG(3) << "entering write_cb()";
    write_cb();
  }
}

void riemann_tcp_connection::read_cb() {
  if (reading_header) {
    read_header();
  } else {
    read_message();
  }
}

void riemann_tcp_connection::write_cb() {
  VLOG(3) << "write_cb()";

  if (bytes_to_write == 0) {
    return; // Do we need this?
  }

  if (!write()) {
    return;
  }

  if (bytes_written == ok_response_size) {
    bytes_to_write = 0;
    VLOG(3) << "response sent";
  }
}

void riemann_tcp_connection::read_header() {
  VLOG(3) << "read_header()";

  if (!read(buffer_size)) {
    return;
  }

  if (bytes_read < 4) {
    return;
  }

  uint32_t header;
  memcpy((void *)&header, r_buffer, 4);
  protobuf_size = ntohl(header);

  VLOG(2) << "header read. protobuf msg size: " << protobuf_size << " bytes";
  if (protobuf_size + 4 > buffer_size) {
    VLOG(2) << "protobuf_size too big: " << protobuf_size;
    close_connection = true;
    return;
  }

  reading_header = false;
  read_message();
}

void riemann_tcp_connection::read_message() {
  VLOG(3) << "read_message()";

  if (bytes_read < protobuf_size + 4) {
    if (!read(protobuf_size + 4)) {
      return;
    }

    if ((bytes_read - 4) != protobuf_size) {
      return;
    }
  }

  reading_header = true;
  bytes_read = 0;
  bytes_to_write = ok_response_size;

  if (bytes_to_write > buffer_size) {
    VLOG(2) << "bytes to write too big: " << bytes_to_write;
    close_connection = true;
    return;
  }

  std::vector<unsigned char> msg(protobuf_size);
  memcpy(&msg[0], r_buffer + 4, protobuf_size);

  income_events.add_undecoded_msg(std::move(msg));
}


