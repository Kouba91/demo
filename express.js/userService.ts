/**
 * Service contains all logic to acquire sequelized models correctly and return them to caller
 * Queries classes are used for sophisticated query methods
 * For simple queries, like findByPk, update, create, etc., direct Model can be used
 */
import securityConfig from '../config/secConfig';
import * as userQueries from '../queries/userQueries';
import User from '../models/User';
import UserDetail from '../models/UserDetail';
const bcrypt = require('bcrypt');

/**
 *
 * @async
 * @param {string} email
 * @param {string} password raw password
 * @returns {Promise<User>} Promise of Sequelized User
 * @description Hashes raw password, using bcrypt and returns Sequelized User
 */
const createUser = async (email: string, password: string) => {
  const hashedPassword = await bcrypt.hash(password, 12);
  const user = { email, password: hashedPassword, firstName: null, lastName: null, phone: null };
  return User.create(user);
};

/**
 *
 * @async
 * @param {User} user Sequelized user with detail
 * @param {string} password raw password
 * @returns {Promise<User>} Promise of sequelized user
 * @description Hashes password and updates user and user detail
 */
const changePassword = async (user: User, password: string) => {
  const hashedPassword = await bcrypt.hash(password, 12);
  await updateDetail(user.userDetail!, {
    resetToken: null,
    resetTokenExpiration: null,
    refreshToken: null,
  });
  return user.update({ password: hashedPassword });
};

/**
 *
 * @param {string} resetToken
 * @param {number} userId
 * @returns {Promise<UserDetail>} Promise of Sequelized UserDetail
 */
const createDetail = (resetToken: string, userId: number) => {
  return UserDetail.create({
    resetToken,
    userId,
    resetTokenExpiration: Date.now() + securityConfig.resetTokenExpiration,
    enabled: false,
    role: 'user',
    refreshToken: null,
  });
};

/**
 *
 * @param {UserDetail} userDetail Sequelized UserDetail
 * @param {object} data properties to be updated
 * @returns {Promise<UserDetail>} Promise of Sequelized UserDetail
 * @description receives Sequelized object, on which update will be performed.
 * All properties to update are specified in 'data'. Updated sequelized object is returned
 */
const updateDetail = (userDetail: UserDetail, data: object) => {
  return userDetail.update(data);
};

/**
 *
 * @param {string} token Reset Token
 * @returns {Promise<UserDetail>} Promise of Sequelized UserDetail
 */
const fetchUserDetailByResetToken = (token: string) => {
  return userQueries.fetchUserDetailByResetToken(token);
};

/**
 *
 * @param {string} email email of user
 * @returns {Promise<User>} Promise of Sequelized User
 */
const fetchUserByEmail = (email: string) => {
  return userQueries.fetchUserByEmail(email);
};

/**
 *
 * @param {number} id id of user
 * @returns {Promise<User>} Promise of Sequelized User
 */
const fetchUserById = (id: number) => {
  return User.findByPk(id);
};

/**
 *
 * @param {string} email email of user
 * @returns {Promise<User,UserDetail>} Promise of Sequelized User with UserDetail
 */
const fetchUserWithDetailByEmail = (email: string) => {
  return userQueries.fetchUserWithDetailByEmail(email);
};

/**
 *
 * @param {string} token Reset Token
 * @returns {Promise<User,UserDetail>} Promise of Sequelized User with UserDetail
 */
const fetchUserWithDetailByResetToken = (token: string) => {
  return userQueries.fetchUserWithDetailByResetToken(token);
};

/**
 *
 * @param {string} token Refresh Token
 * @returns {Promise<User,UserDetail>} Promise of Sequelized User with UserDetail
 */
const fetchUserWithDetailByRefreshToken = (token: string) => {
  return userQueries.fetchUserWithDetailByRefreshToken(token);
};

export {
  createUser,
  changePassword,
  createDetail,
  fetchUserById,
  fetchUserDetailByResetToken,
  fetchUserWithDetailByResetToken,
  updateDetail,
  fetchUserByEmail,
  fetchUserWithDetailByEmail,
  fetchUserWithDetailByRefreshToken,
};
