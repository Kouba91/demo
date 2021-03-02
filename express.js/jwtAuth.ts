import securityConfig from '../config/secConfig';
import { Request, Response, NextFunction } from 'express';
const jwt = require('jsonwebtoken');

/**
 *
 * @param {Request} req
 * @param {Response} res
 * @param {next} next
 * @description Middleware function, that intercepts all URL with /api (excludes /api/auth).
 * Verifies JSONWebToken. If valid, token payload is set to req.principal (userId and role).
 * Request is allowed to continue to requested URL. If not valid,
 * response is sent back. If token is valid, but expired, message 'JWT expired' is sent in response.
 * Next request, after expired token request, from the same source, must include this expired, but valid
 * token, 'RefreshToken' header, and requeste URL must be /api/refreshtoken.
 * If these conditions are met, token is decoded and its payload is set to req.tokenPayload
 * (userId, role, iat, eat) and request continues to /api/refreshtoken
 */
export default (req: Request, res: Response, next: NextFunction) => {
  const { authorization } = req.headers;

  if (!authorization) {
    return res.status(401).send({ error: 'You must be logged in.' });
  }

  const token = authorization.replace('Bearer ', '');
  type jwtToken = { userId: number; role: 'user' | 'admin' };
  jwt.verify(token, securityConfig.jwtSecret, async (err: Error, payload: jwtToken) => {
    if (err) {
      switch (err.name) {
        case 'TokenExpiredError':
          //allow for token refresh, if refresh token is present
          if (req.url == '/refreshtoken' && req.headers.refreshtoken) {
            const tokenPayload = jwt.decode(token);
            const { userId, role } = payload;
            req.principal = { userId, role };
            return next();
          } else return res.status(401).send({ error: 'JWT expired' });
      }

      return res.status(401).send({ error: 'You must be logged in.' });
    }
    //send userId and role with every request as principal
    const { userId, role } = payload;
    req.principal = { userId, role };

    next();
  });
};
