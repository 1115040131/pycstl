const { v4: uuidv4 } = require('uuid');

const grpc = require('@grpc/grpc-js');

const config_module = require('./src/config');
const const_module = require('./src/const');
const emailModule = require('./src/email');
const verify_proto = require('./src/proto');
const redis_module = require('./src/redis');

async function GetVerifyCode(call, callback) {
    console.log('email is', call.request.email);
    try {
        let key = const_module.code_prefix + call.request.email;

        let query_res = await redis_module.QueryRedis(key);
        console.log('query_res is', query_res);
        let uniqueId = query_res;
        if (query_res == null) {
            uniqueId = uuidv4();
            if (uniqueId.length > 4) {
                uniqueId = uniqueId.substr(0, 4);
            }
            let bres = await redis_module.SetRedisExpire(key, uniqueId, 180);
            if (!bres) {
                callback(null, {
                    email: call.request.email,
                    error: const_module.Errors.RedisErr
                });
                return;
            }
        } else {
            let get_res = await redis_module.GetRedis(key);
            console.log('get_res is', get_res);
            if (get_res == null) {
                callback(null, {
                    email: call.request.email,
                    error: const_module.Errors.RedisErr
                });
                return;
            }
            uniqueId = get_res;
        }
        console.log('uniqueId is', uniqueId);

        let text_str = '验证码：' + uniqueId + ', 请在3分钟内使用';
        // 发送邮件
        let mailOptions = {
            from: config_module.email_user,
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };

        let send_res = await emailModule.SendMail(mailOptions);
        console.log('send res is', send_res);

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Success
        });
    } catch (error) {
        console.log('error is', error);

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Exception
        });
    }
}

function main() {
    var server = new grpc.Server();
    server.addService(verify_proto.VerifyService.service, { GetVerifyCode: GetVerifyCode });
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        // server.start();
        console.log('grpc server started')
    });
}

main()