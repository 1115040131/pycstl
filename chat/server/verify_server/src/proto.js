const grpc = require('@grpc/grpc-js')
const protoLoader = require('@grpc/proto-loader')

const PROTO_PATH = 'chat/server/proto/verify.proto'
const packageDefinition = protoLoader.loadSync(PROTO_PATH, {
    KeepCase: true, longs: String, enum: String, defaults: true, oneofs: true
})

const protoDescriptor = grpc.loadPackageDefinition(packageDefinition)

const verify_proto = protoDescriptor.pyc.chat

module.exports = verify_proto